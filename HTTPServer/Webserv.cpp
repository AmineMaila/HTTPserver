#include "Webserv.hpp"
#include "../Server/ServerHandler.hpp"
#include "../Client/CGI/CGIHandler.hpp"

bool	Webserv::running = true;

Webserv::~Webserv()
{
	while (!handlerMap.empty())
		cleanup(handlerMap.begin()->second);
	close(epoll_fd);
}

Webserv::Webserv(std::vector<ServerConfig>& servers) : servers(servers)
{
	cgiCounter = 0;
	srand(std::time(0));
	epoll_fd = epoll_create1(0);
}

void	Webserv::eraseDependency(EventHandler *dependent) {dependencyMap.erase(dependent);}

void	Webserv::stop() { running = false; }

void	Webserv::addTimer(EventHandler *client) { clientTimer.push_back(std::make_pair(client, std::time(NULL))); }

void	Webserv::incCgiCounter() {++cgiCounter;}

void	Webserv::decCgiCounter() {--cgiCounter;}

int		Webserv::getCgiCounter() const {return (cgiCounter);}

void	Webserv::collect(EventHandler *handler) { deleted.push_back(handler); }

void	Webserv::registerDependency(EventHandler *dependent, EventHandler *dependency) { dependencyMap[dependent] = dependency; }

void	Webserv::updateTimer(EventHandler *client)
{
	std::vector<std::pair<EventHandler *, time_t> >::iterator it;
	for (it = clientTimer.begin(); it != clientTimer.end(); it++)
	{
		if (it->first == client)
		{
			it->second = std::time(NULL);
			break;
		}
	}
}

void	Webserv::eraseTimer(EventHandler *client)
{
	std::vector<std::pair<EventHandler *, time_t> >::iterator it;
	for (it = clientTimer.begin(); it != clientTimer.end(); it++)
	{
		if (it->first == client)
		{
			clientTimer.erase(it);
			break;
		}
	}
}

void	Webserv::registerHandler(int fd, EventHandler *handler, uint32_t events)
{
	struct epoll_event ev;

	ev.events = events;
	ev.data.ptr = handler;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
	handlerMap.insert(std::make_pair(fd, handler));
	handler->HTTPserver = this;
}

void	Webserv::updateHandler(const int fd, uint32_t events)
{
	struct epoll_event ev;

	ev.events = events;
	ev.data.ptr = handlerMap[fd];
	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}

void	Webserv::removeHandler(int fd)
{
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	std::map<int, EventHandler*>::iterator it = handlerMap.find(fd);
	if (it != handlerMap.end())
		handlerMap.erase(it);
	close(fd);
}

int	Webserv::bindSocket(std::string& host, std::string& port)
{
	struct addrinfo		hints;
	
	// initialize addrinfo
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	struct addrinfo	*res;
	int status = getaddrinfo(host.c_str(), port.c_str(), &hints, &res);
	if (status != 0)
	{
		std::cerr << YELLOW << "\tWebserv: getaddrinfo: " << gai_strerror(status) << RESET << std::endl;
		return (-1);
	}

	struct addrinfo		*it;
	int					serverSocket = -1;

	for (it = res; it; it = it->ai_next)
	{
		serverSocket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
		if (serverSocket == -1)
			continue;

		int yes = 1;
		if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes)) == -1)
		{
			std::cerr << YELLOW << "\tWebserv: setsocketopt: " << strerror(errno) << RESET << std::endl;
			close(serverSocket);
			freeaddrinfo(res);
			return (-1);
		}

		if (bind(serverSocket, it->ai_addr, it->ai_addrlen) == 0)
			break;
		close(serverSocket);
	}
	freeaddrinfo(res);
	if (!it)
	{
		close(serverSocket);
		std::cerr << YELLOW << "\tWebserv : Failed to Bind Any Socket..." << RESET << std::endl;
		return (-1);
	}
	return (serverSocket);
}

int    Webserv::listenForConnections(int& serverSocket)
{
	if (listen(serverSocket, BACKLOG) == -1)
	{
		std::cerr << YELLOW << "\tWebserv : listen: " << strerror(errno) << RESET << std::endl;
		close(serverSocket);
		return (-1);
	}
	if (fcntl(serverSocket, F_SETFD, FD_CLOEXEC) == -1)
	{
		std::cerr << YELLOW << "\tWebserv : fcntl: " << strerror(errno) << RESET << std::endl;
		close(serverSocket);
		return (-1);
	}
	return (0);
}

bool	Webserv::initServers()
{
	int													serverSocket;
	std::vector<ServerConfig>::iterator					it;
	std::map<std::pair<std::string, std::string>, int>	boundServers;

	for (it = servers.begin(); it != servers.end(); it++)
	{
		std::pair<std::string, std::string> bindAddress = std::make_pair(it->host, it->port);
		std::map<std::pair<std::string, std::string>, int>::iterator deja = boundServers.find(bindAddress);

		if (deja != boundServers.end())
		{
			static_cast<ServerHandler *>(handlerMap[deja->second])->addVServer(*it);
			continue ;
		}

		serverSocket = bindSocket(it->host, it->port);
		if (serverSocket == -1)
			continue ;
		if (listenForConnections(serverSocket) == -1)
		{
			close(serverSocket);
			continue ;
		}
			
		ServerHandler	*handler = new (std::nothrow) ServerHandler(serverSocket);
		if (!handler)
		{
			close(serverSocket);
			continue ;
		}
		handler->addVServer(*it);
		boundServers.insert(std::make_pair(bindAddress, serverSocket));
		registerHandler(serverSocket, handler, EPOLLIN);
		std::cout << BLUE << BOLD << "Server listening on " << it->host << ":" << it->port << RESET << std::endl;
	}
	return (!boundServers.empty());
}

void	Webserv::clientTimeout()
{
	time_t now = std::time(NULL);

	for (timeIt = clientTimer.begin(); timeIt != clientTimer.end(); )
	{
		ClientHandler *client = static_cast<ClientHandler *>(timeIt->first);
		if (client->childStatus())
		{
			timeIt++;
			continue;
		}

		if (now - timeIt->second >= TIMEOUT)
		{
			timeIt = clientTimer.erase(timeIt);
		
			if (client->getCgiActive())
				client->kickCGI(504);
			else
				delete client;
		}
		else if (client->getCgiActive() && now - client->getCgiTimer() >= CGI_TIMEOUT)
		{
			timeIt = clientTimer.erase(timeIt);
			delete client;
		}
		else
			++timeIt;
	}
}

void	Webserv::cleanup(EventHandler *handler)
{
	if (!handler)
		return;

	std::map<EventHandler *, EventHandler *>::iterator it;
	it = dependencyMap.find(handler);
	if (it != dependencyMap.end())
		delete it->second; // deleting the dependency deletes the dependent in the destructor
	else
		delete handler;
}

void	Webserv::run()
{
	struct epoll_event events[MAX_EVENTS];
	while (running)
	{
		int eventCount = epoll_wait(epoll_fd, events, MAX_EVENTS, EPOLL_TIMEOUT);

		for (int i = 0; i < eventCount; i++)
		{
			EventHandler	*handler = static_cast<EventHandler *>(events[i].data.ptr);
			if (std::find(deleted.begin(), deleted.end(), handler) != deleted.end())
				continue;

			try
			{
				if (events[i].events & EPOLLERR)
					cleanup(handler);
				else
					handler->handleEvent(events[i].events);
			}
			catch (Disconnect& e)
			{
				std::cerr << YELLOW << e.what() << RESET << std::endl;
				cleanup(handler);
			}
		}
		deleted.clear();
		clientTimeout();
	}
}
