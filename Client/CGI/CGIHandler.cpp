#include "CGIHandler.hpp"
#include "../../HTTPServer/Webserv.hpp"
#include "../Response/Error.hpp"

CGIHandler::~CGIHandler()
{
	close(outfd);
	close(infd);
	if (waitpid(pid, NULL, WNOHANG) == 0)
		kill(pid, SIGTERM);
}

CGIHandler::CGIHandler(int& clientSocket, RequestData *data) : Response(clientSocket, data), outfd(-1), infd(-1), pid(0), parseBool(true), CGIState(LENGTH)
{
	int pipe_fd[2];
	if (pipe(pipe_fd) == -1)
		throw(FatalError(strerror(errno)));
	outfd = pipe_fd[0];
	infd = pipe_fd[1];
	// state = PARSE;
}

pid_t CGIHandler::getPid() const
{
	return (pid);
}

std::string	headerToEnv(const std::string& header)
{
	std::string envVar = const_cast<std::string&>(header);

	if (header != "content-type" && header != "content-length")
		envVar.insert(0, "HTTP_");
	
	envVar.replace(envVar.begin(), envVar.end(), '-', '_');
	for (size_t i = 0; i < envVar.size(); i++)
		toupper(envVar[i]);
	envVar.append("=");
	return (envVar);
}

void	CGIHandler::buildEnv()
{

	// envvars.push_back("SERVER_NAME=" + reqCtx->config.);
	envvars.push_back("REQUEST_METHOD=" + reqCtx->Method);
	envvars.push_back("SCRIPT_NAME=" + reqCtx->scriptName);
	envvars.push_back("PATH_INFO=" + reqCtx->pathInfo);
	envvars.push_back("QUERY_STRING=" + reqCtx->queryString);
	envvars.push_back("SERVER_SOFTWARE=webserv/1.0");
	envvars.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envvars.push_back("SERVER_PROTOCOL=HTTP/1.1");

	// headers to ENV
	std::map<std::string, std::string>::iterator header;
	for (header = reqCtx->Headers.begin(); header != reqCtx->Headers.end(); header++)
	{
		envvars.push_back(headerToEnv(header->first) + header->second);
	}
}

int	CGIHandler::setup()
{
	pid = fork();
	if (pid == -1)
	{
		close(outfd);
		close(infd);
		throw(FatalError(strerror(errno)));

	}
	else if (pid == 0)
	{
		if (dup2(infd, 1) == -1)
		{
			std::cerr << "[WEBSERV]\t";
			perror("dup2");
			close(outfd);
			close(infd);
			exit(errno);
		}
		close(outfd);
		close(infd);

		std::string dir = reqCtx->fullPath.substr(0, reqCtx->fullPath.find(reqCtx->scriptName));
		if (chdir(dir.c_str()) == -1)
		{
			std::cerr << "[WEBSERV]\t";
			perror("chdir");
			exit(errno);
		}

		char *arg[3];
		arg[0] = const_cast<char *>(reqCtx->cgiIntrepreter.c_str());
		arg[1] = const_cast<char *>(reqCtx->scriptName.c_str());
		arg[2] = NULL;

		buildEnv();
		std::vector<char *>	env;

		for (std::vector<std::string>::iterator it = envvars.begin(); it != envvars.end(); it++)
		{
			env.push_back(const_cast<char *>(it->c_str()));
		}
		env.push_back(NULL);

		if (execve(arg[0], arg, env.data()) == -1)
		{
			std::cerr << "[WEBSERV][ERROR]\t";
			perror("execve");
			exit(errno);
		}
	}
	close(infd);
	return (outfd);
}

void	CGIHandler::processCGIHeaders(std::map<std::string, std::string>& headersMap)
{
	if (headersMap.find("content-type") != headersMap.end())
	{
		headers.append("\r\nContent-Type: " + headersMap["content-type"]);
		headersMap.erase("content-type");
	}
	else if (!buffer.empty()) // RFC 3875 section 6.3.1
	{
		std::cerr << "[WEBSERV][ERROR]\tContet-Type Header Was Not Found In CGI" << std::endl;
		throw(500);
	}

	if (headersMap.find("content-length") != headersMap.end())
	{
		headers.append("\r\nContent-Length: " + headersMap["content-length"]);
		headersMap.erase("content-length");
	}
	else
	{
		headers.append("\r\nTransfer-Encoding: chunked");
		CGIState = CHUNKED;
	}

	std::string location;
	if (headersMap.find("location") != headersMap.end())
	{
		location = headersMap["location"];
		if (location.at(0) == '/')
			throw(CGIRedirectException(location));
		headersMap.erase("location");
	}
	if (headersMap.find("status") != headersMap.end())
	{
		std::string statusStr = headersMap["status"];
		headersMap.erase("status");

		size_t numStart = statusStr.find_first_not_of(" \t\f\v");
		size_t numEnd = statusStr.find_first_of(" \t\f\v", numStart);
		char *end;
		std::string numStr = statusStr.substr(numStart, numEnd - numStart);
		int statusCode = strtoul(numStr.c_str(), &end, 10);
		if (errno == ERANGE || *end)
		{
			std::cerr << "[WEBSERV][ERROR]\tMalformed Status Header In CGI" << std::endl;
			throw(500);
		}
		statusStr.erase(0, numEnd);
		statusStr = stringtrim(statusStr, " \t\f\v");
		if (statusCode == 302 && (location.empty() || buffer.empty())) // RFC 3875 section 6.3.3
		{
			std::cerr << "[WEBSERV][ERROR]\t302 Status Code With No Location Header" << std::endl;
			throw(500);
		}
		else
			headers.append("\r\nLocation: " + location);
		headers.insert(0, "HTTP/1.1 " + _toString(statusCode) + " " + statusStr);
	}
	else
		headers.insert(0, "HTTP/1.1 " + _toString(reqCtx->StatusCode) + " " + statusCodes[reqCtx->StatusCode]);
	
	for (std::map<std::string, std::string>::iterator itr = headersMap.begin(); itr != headersMap.end(); itr++)
	{
		std::string key = itr->first;
		capitalize(key);
		headers.append("\r\n" + key + ": " + itr->second);
	}

	headers.append("\r\n\r\n");
}

void	CGIHandler::parseCGIHeaders()
{
	size_t pos = buffer.find("\r\n\r\n");
	if (pos == std::string::npos)
	{
		std::cerr << "[WEBSERV][ERROR]\tCGI Headers Not Found" << std::endl;
		throw(500);
	}

	headers.append("\r\nServer: webserv/1.0");
	headers.append("\r\nDate: " + getDate());
	if (reqCtx->keepAlive)
		headers.append("\r\nConnection: keep-alive");
	else
		headers.append("\r\nConnection: close");
	
	std::stringstream headerStream(buffer.substr(0, pos + 2));
	buffer.erase(0, pos + 4);

	std::map<std::string, std::string>	headersMap;
	std::string field;

	while (getline(headerStream, field, '\n'))
	{
		if (field.empty())
			break;
		
		if (field[field.size() - 1] == '\r')
        	field.erase(field.size() - 1);
    
	
		std::cout << "Field :::::::" << field << "::::::: End_Field" << std::endl;
		size_t pos = field.find(':');
		if (pos == std::string::npos)
		{
			std::cerr << "[WEBSERV][ERROR]\tMalformed CGI Headers" << std::endl;
			throw(500);
		}
		std::string key = stringtolower(field.substr(0, pos));
		std::string value = stringtrim(field.substr(pos + 1), " \t\v\f");
		if (value.empty())
			continue ; // RFC 3875 section 6.3
		if (key.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-") != std::string::npos)
		{
			std::cerr << "[WEBSERV][ERROR]\tMalformed Headers Field-Name" << std::endl;
			throw(500);
		}
		headersMap[key] = value;
	}

	processCGIHeaders(headersMap);
	parseBool = false;
	std::cout << "_______" << std::endl;
	std::cout << headers ;
	std::cout << "_______" << std::endl;
}

void		CGIHandler::readChunked()
{
	char	buf[SEND_BUFFER_SIZE] = {0};
	int		bytesRead = read(outfd, buf, SEND_BUFFER_SIZE);
	if (bytesRead == -1)
	{
		throw(FatalError(strerror(errno)));
	}
	if (bytesRead > 0)
	{
		// buffer = std::string(buf, bytesRead);
		buffer = buildChunk(buf, bytesRead);
		std::cout << YELLOW << "======[READ DATA OF SIZE " << bytesRead << "]======" << RESET << std::endl;
		state = WRITE;
	}
	else
	{
		buffer = buildChunk(NULL, 0);
		state = WRITE;
		nextState = DONE;
	}
}

void		CGIHandler::readLength()
{
	char	buf[SEND_BUFFER_SIZE] = {0};
	int		bytesRead = read(outfd, buf, SEND_BUFFER_SIZE);
	if (bytesRead == -1)
	{
		throw(FatalError(strerror(errno)));
	}
	if (bytesRead > 0)
	{
		buffer = std::string(buf, bytesRead);
		std::cout << YELLOW << "======[READ DATA OF SIZE " << bytesRead << "]======" << RESET << std::endl;
		state = WRITE;
	}
	else
	{
		state = DONE;
	}
}

int		CGIHandler::feedCgi(const char *buf)
{
	int		bytesWritten = write(infd, buf, SEND_BUFFER_SIZE);
	if (bytesWritten == -1)
	{
		throw(FatalError(strerror(errno)));
	}
	return (bytesWritten);
}

void	CGIHandler::handleEvent(uint32_t events)
{
	if ((events & EPOLLIN || events & EPOLLHUP) && state != DONE)
	{
		switch (CGIState)
		{
			case CHUNKED:
				readChunked();
				break;
			case LENGTH:
				readLength();
				break;
		}
		HTTPserver->updateHandler(socket, EPOLLOUT | EPOLLHUP);
		HTTPserver->updateHandler(outfd, 0);
	}
	// else if (events& EPOLLOUT)
	// {
	// 	feedCgi();
	// }
}

int		CGIHandler::respond()
{
	switch (state)
	{
		case READ:
			HTTPserver->updateHandler(socket, EPOLLHUP);
			HTTPserver->updateHandler(outfd, EPOLLIN | EPOLLHUP);
			break;
		case WRITE:
			if (parseBool)
				parseCGIHeaders();
			if ((this->*sender)() == true)
			{
				if (buffer.empty())
					state = nextState;
				else if (CGIState == CHUNKED)
					buffer = buildChunk(buffer.c_str(), buffer.size());
			}
			break;
		case DONE:
			return (1);
	}
	return (0);
}