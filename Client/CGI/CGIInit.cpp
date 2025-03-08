#include "CGIHandler.hpp"
#include "../../HTTPServer/Webserv.hpp"

std::string headerToEnv(const std::string& headerKey, const std::string& headerValue)
{
    std::string envVar = "HTTP_";
    envVar.reserve(headerKey.size() + headerValue.size() + 7); // Pre-allocate memory
    
    for (size_t i = 0; i < headerKey.size(); i++)
        envVar += (headerKey[i] == '-') ? '_' : std::toupper(headerKey[i]);
    
    return (envVar + "=" + headerValue);
}

void	CGIHandler::buildEnv()
{
	envVars.reserve(reqCtx->Headers.size() + 12); // 12 the number of env vars not in headers

	envVars.push_back("REQUEST_METHOD=" + reqCtx->Method);
	envVars.push_back("SCRIPT_NAME=" + reqCtx->scriptName);
	envVars.push_back("PATH_INFO=" + reqCtx->pathInfo);
	envVars.push_back("QUERY_STRING=" + reqCtx->queryString);
	envVars.push_back("PATH_TRANSLATED="+ reqCtx->pathTranslated);
	envVars.push_back("REQUEST_URI=" + reqCtx->URI);
	envVars.push_back("SERVER_HOST=" + reqCtx->serverHost);
	envVars.push_back("SERVER_PORT=" + reqCtx->serverPort);
	envVars.push_back("SERVER_SOFTWARE=webserv/1.0");
	envVars.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envVars.push_back("SERVER_PROTOCOL=HTTP/1.1");

	if (reqCtx->isEncoded && !reqCtx->tmpFileName.empty())
	{
		ssize_t fileSize = fileLength(reqCtx->tmpFileName);
		if (fileSize == -1)
			exit(errno);
		envVars.push_back("CONTENT_LENGTH=" + _toString(fileSize));
	}

	// headers to ENV
	std::map<std::string, std::string>::iterator header;
	for (header = reqCtx->Headers.begin(); header != reqCtx->Headers.end(); header++)
	{
		if (header->first == "content-type")
			envVars.push_back("CONTENT_TYPE=" + header->second);
		else if (!reqCtx->isEncoded && header->first == "content-length")
			envVars.push_back("CONTENT_LENGTH=" + header->second);
		else
			envVars.push_back(headerToEnv(header->first, header->second));
	}

	envPtr.reserve(envVars.size() + 1);

	for (std::vector<std::string>::iterator it = envVars.begin(); it != envVars.end(); it++)
	{
		envPtr.push_back(const_cast<char *>(it->c_str()));
	}
	envPtr.push_back(NULL);
}

void	CGIHandler::execCGI()
{
	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1)
	{
		std::cerr << YELLOW << "\tCGI : socketpair : " << strerror(errno) << RESET << std::endl;
		throw(Code(500));
	}
	int fd = -1;
	if (!reqCtx->tmpFileName.empty())
	{
		fd = open(reqCtx->tmpFileName.c_str(), O_RDONLY);
		if (fd == -1)
		{
			std::cerr << YELLOW << "\tCGI : open : " << strerror(errno) << RESET << std::endl;
			close(sv[0]);
			close(sv[1]);
			throw(Code(500));
		}
	}

	if (access(reqCtx->cgiIntrepreter.c_str(), F_OK | X_OK) != 0)
		throw(Code(500));

	pid = fork();
	if (pid == -1)
	{
		std::cerr << YELLOW << "\tCGI : fork : " << strerror(errno) << RESET << std::endl;
		close(sv[0]);
		close(sv[1]);
		throw(Code(500));

	}
	else if (pid == 0)
	{
		close(sv[0]);
		if (dup2(sv[1], STDOUT_FILENO) == -1)
		{
			std::cerr << YELLOW << "\tCGI : dup2 : " << strerror(errno) << RESET << std::endl;
			close(sv[1]);
			if (fd != -1)
				close(fd);
			exit(errno);
		}

		if (fd != -1)
		{
            if (dup2(fd, STDIN_FILENO) == -1) {
                std::cerr << YELLOW << "\tCGI : dup2 : " << strerror(errno) << RESET << std::endl;
                close(sv[1]);
                close(fd);
				exit(errno);
            }
            close(fd);
        }
		else if (dup2(sv[1], STDIN_FILENO) == -1)
		{
            std::cerr << YELLOW << "\tCGI : dup2 : " << strerror(errno) << RESET << std::endl;
            close(sv[1]);
			exit(errno);
        }

		close(sv[1]);
		std::string dir = reqCtx->fullPath.substr(0, reqCtx->fullPath.find_last_of("/"));
		if (chdir(dir.c_str()) == -1)
		{
			std::cerr << YELLOW << "\tCGI : chdir : " << strerror(errno) << RESET << std::endl;
			exit(errno);
		}

		args[0] = const_cast<char *>(reqCtx->cgiIntrepreter.c_str());
		args[1] = const_cast<char *>(reqCtx->fileName.c_str());
		args[2] = NULL;

		buildEnv();
		if (execve(args[0], args, envPtr.data()) == -1)
		{
			std::cerr << YELLOW << "\tCGI : execve : " << strerror(errno) << RESET << std::endl;
			exit(errno);
		}
	}
	if (fd != -1) close(fd);
	close(sv[1]);
	cgiSocket = sv[0];
}
