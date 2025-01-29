#include "ClientHandler.hpp"

ClientHandler::~ClientHandler()
{
	if (cgi)
		delete cgi;
	delete this;
}

ClientHandler::ClientHandler() : cgi(NULL) {}

ClientHandler::ClientHandler(int fd, std::vector<ServerConfig> vServers) : socket(fd), vServers(vServers) {}

void	ClientHandler::reset()
{
	response = Response();
	// request = Request();
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
		input.absolutePath.append(URL.substr(0, pos));
		URL.erase(0, pos++);
		if (stat(input.absolutePath.c_str(), &pathStat) == -1)
		{
			input.status = 404;
			return ;
		}
		if (!S_ISDIR(pathStat.st_mode))
			break;
	}

	if (S_ISDIR(pathStat.st_mode))
	{
		if (access(input.absolutePath.c_str(), X_OK) != 0)
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
				if (access((input.absolutePath + *it).c_str(), F_OK) == 0) // file exists
				{
					input.absolutePath.append(*it);
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
		std::cout << "PATH AFTER IS DIR: >>> " << input.absolutePath << std::endl;
	}

	if (!input.isDir)
	{	
		if (access(input.absolutePath.c_str(), R_OK) != 0)
		{
			input.status = 403;
			return ;
		}

		std::string scriptName = input.absolutePath.substr(input.absolutePath.find_last_of('/') + 1);
		if ((pos = scriptName.find('.')) != std::string::npos)
		{
			std::map<std::string, std::string>::iterator it = input.config.cgi_ext.find(scriptName.substr(pos));
			if (it != input.config.cgi_ext.end())
			{
				cgi = new CGIHandler(it->second, it->first, scriptName, URL, queryString);
				cgi->setup();
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
		// request
	}
	else if (events & EPOLLOUT)
	{
		int retVal = response.sendResponse(socket);
		if(retVal == -1) // can be || (retval == 1 && client.(connection header is "close"))
		{
			HTTPserver->removeHandler(socket);
			std::cout << "CLIENT REMOVED" << std::endl;
		}
		else if (retVal == 1)
		{
			std::cout << "CLIENT " << socket << " SERVED. RESETING.." << std::endl;
			HTTPserver->updateHandler(socket, EPOLLIN | EPOLLHUP);
			reset();
		}	
	}
}
