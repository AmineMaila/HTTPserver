#include "ClientHandler.hpp"
#include "CGI/CGIHandler.hpp"
#include "../HTTPServer/Webserv.hpp"

ClientHandler::~ClientHandler()
{
	delete this;
}

ClientHandler::ClientHandler(int fd, std::vector<ServerConfig> vServers) : socket(fd), vServers(vServers), cgifd(-1) {}

void	ClientHandler::reset()
{
	response = Response();
	request = Request();
}

// void	ClientHandler::setResponseBuffer(std::string buffer)
// {
// 	response.setBuffer(buffer);
// }

int	ClientHandler::getSocket() const
{
	return (socket);
}

ServerConfig&	ClientHandler::matchingServer(std::string& host)
{
	for (std::vector<ServerConfig>::iterator it = vServers.begin(); it != vServers.end(); it++)
	{
		if (it->host == host)
			return (*it);
	}
	return (vServers[0]);
}

std::string	queryString(std::string& path) // protocol://domain/path/script.cgi/pathinfo?query#fragment
{
	std::string str;
	
	// remove the fragment part from the uri
	size_t pos = path.find('#');
	if (pos != std::string::npos)
		path.erase(pos);

	// extracts the query string from the uri
	pos = path.find('?');
	if (pos != std::string::npos)
	{
		str = path.substr(pos + 1);
		path.erase(pos);
	}
	return (str);
}

void	ClientHandler::decodeUri(struct ResponseInput& input, std::string& URL)
{
	if (!rootJail(input.uri))
	{
		input.status = 403;
		return ;
	}

	struct stat pathStat;
	size_t 		pos;

	std::string	queryString;
	pos = URL.find_last_of('#');
	if (pos != std::string::npos)
		URL.erase(pos);
	pos = URL.find_last_of('?');
	if (pos != std::string::npos)
	{
		queryString = URL.substr(pos + 1);
		URL.erase(pos);
	}

	while (!URL.empty())
	{
		pos = URL.find('/', 1);
		input.path.append(URL.substr(0, pos));
		URL.erase(0, pos++);
		if (stat(input.path.c_str(), &pathStat) == -1)
		{
			input.status = 404;
			return ;
		}
		if (!S_ISDIR(pathStat.st_mode))
			break;
	}

	// append index
	if (S_ISDIR(pathStat.st_mode))
	{
		if (access(input.path.c_str(), X_OK) != 0)
		{
			input.status = 403;
			return ;
		}
		input.isDir = true;
		if (input.method == "GET")
		{
			std::vector<std::string>::iterator	it;

			for (it = input.config.index.begin(); it != input.config.index.end(); it++)
			{
				if (access((input.path + *it).c_str(), F_OK) == 0) // file exists
				{
					input.path.append(*it);
					input.isDir = false;
					break;
				}
			}
			if (it == input.config.index.end())
			{
				if (!input.config.autoindex)
					input.status = 404;
				return;
			}
		}
		std::cout << "PATH AFTER IS DIR: >>> " << input.path << std::endl;
	}

	if (!input.isDir)
	{	
		if (access(input.path.c_str(), R_OK) != 0)
		{
			input.status = 403;
			return ;
		}

		std::string scriptName = input.path.substr(input.path.find_last_of('/') + 1);
		if ((pos = scriptName.find('.')) != std::string::npos)
		{
			std::map<std::string, std::string>::iterator it = input.config.cgi_ext.find(scriptName.substr(pos));
			if (it != input.config.cgi_ext.end())
			{
				std::vector<std::string> envVars;
		
				envVars.push_back("REQUEST_METHOD=" + input.method);
				envVars.push_back("SCRIPT_NAME=" + scriptName);
				envVars.push_back("PATH_INFO=" + URL);
				envVars.push_back("QUERYSTRING=" + queryString);
				envVars.push_back("SERVER_PROTOCOL=HTTP/1.1");
				// needs headers to be formatted as env vars;
				std::vector<char *>	env;

				for (std::vector<std::string>::iterator it = envVars.begin(); it != envVars.end(); it++)
				{
					env.push_back(const_cast<char *>(it->c_str()));
				}
				env.push_back(NULL);

				char *arg[3];
				arg[0] = const_cast<char *>(it->second.c_str());
				arg[1] = const_cast<char *>(scriptName.c_str());
				arg[2] = NULL;

				CGIHandler	*cgi = new CGIHandler(socket, input.path, arg, env.data());

				if((cgifd = cgi->setup()) == -1)
				{
					input.status = 500;
					delete cgi;
					return;
				}
				HTTPserver->registerHandler(cgi->getFd(), cgi, EPOLLIN | EPOLLHUP);
			}
		}
		else if (URL.size() > 0)
		{
			input.status = 404;
			return;
		}
	}
}

void	ClientHandler::initResponse()
{
	struct ResponseInput				input;

	input.method = request.getRequestLineSt().method;
	input.uri = request.getRequestLineSt().uri;
	input.status = request.getStatusCode();
	input.requestHeaders = request.getHeaderSt().headersMap;
	
	ServerConfig server = matchingServer(request.getHeaderSt().host);

	std::string	location;
	std::map<std::string, Directives>::iterator it;
	for (it = server.Locations.begin(); it != server.Locations.end(); it++)
	{
		if (input.uri.find(it->first) != std::string::npos)
		{
			if (it->first.size() > location.size())
				location = it->first;
		}
	}
	if (location.empty())
		input.config = server.ServerDirectives;
	else
		input.config = server.Locations.find(location)->second;


	std::string	URL;
	// alias and root appending
	if (!input.config.alias.empty())
		URL = input.config.alias + input.uri.substr(location.length());
	else
	{
		if (input.config.root[input.config.root.length() - 1] == '/') // do same for alias
			input.config.root.erase(input.config.root.length() - 1);
		URL = input.config.root + input.uri;
	}
	
	input.config.cgi_ext.insert(std::make_pair(".py", "/usr/bin/python3"));
	input.config.cgi_ext.insert(std::make_pair(".php", "/usr/bin/php"));
	input.config.index.push_back("index.html");
	input.config.autoindex = true;
	decodeUri(input, URL);
	response.setInput(input);
}

void	ClientHandler::handleEvent(uint32_t events)
{
	if (events & EPOLLIN)
	{
		int state = request.receiveRequest(socket);
		if (state == PARSING_FINISHED)
		{
			HTTPserver->updateHandler(socket, EPOLLOUT | EPOLLHUP);
			initResponse();
		}
		else if (state == -1)
		{
			HTTPserver->removeHandler(socket);
		}

	}
	else if (events & EPOLLOUT)
	{
		int state = response.sendResponse(socket);
		if(state == -1) // can be || (state == 1 && client.(connection header is "close"))
		{
			if (cgifd != -1)
				HTTPserver->removeHandler(cgifd);
			HTTPserver->removeHandler(socket);
			std::cout << "CLIENT REMOVED" << std::endl;
		}
		else if (state == 1)
		{
			std::cout << "CLIENT " << socket << " SERVED. RESETING.." << std::endl;
			HTTPserver->updateHandler(socket, EPOLLIN | EPOLLHUP);
			reset();
		}
	}
}
