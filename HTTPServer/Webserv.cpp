#include "Webserv.hpp"
#include "../Server/ServerHandler.hpp"

Webserv::~Webserv() {}

Webserv::Webserv(std::vector<ServerConfig>& servers) : servers(servers)
{
	epoll_fd = epoll_create1(0);
}

void print_epoll_events(uint32_t events)
{
	if (events & EPOLLIN)  std::cout << "EPOLLIN ";
	if (events & EPOLLOUT) std::cout << "EPOLLOUT ";
	if (events & EPOLLRDHUP) std::cout << "EPOLLRDHUP ";
	if (events & EPOLLPRI) std::cout << "EPOLLPRI ";
	if (events & EPOLLERR) std::cout << "EPOLLERR ";
	if (events & EPOLLHUP) std::cout << "EPOLLHUP ";
	if (events & EPOLLET) std::cout << "EPOLLET ";
	if (events & EPOLLONESHOT) std::cout << "EPOLLONESHOT ";
}

void	Webserv::registerHandler(int fd, EventHandler *handler, uint32_t events)
{
	struct epoll_event ev;

	ev.events = events;
	ev.data.ptr = handler;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
	handlerMap[fd] = handler;
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
	close(fd);
	std::map<int, EventHandler*>::iterator it = handlerMap.find(fd);
	if (it != handlerMap.end())
	{
		EventHandler	*handler = it->second;
		handlerMap.erase(it);
		delete handler;
	}
	if (handlerMap.size() == 0)
	{
		std::cerr << "[WEBSERV] No Servers Left. Exiting..." << std::endl;
		exit(1);
	}
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
		std::cerr << "[WEBSERV]\t> getaddrinfo: " << gai_strerror(status) << std::endl;
		exit(errno);
	}

	// create socket and assign (bind) that socket an address returned in res
	struct addrinfo		*it;
	int					serverSocket;

	for (it = res; it; it = it->ai_next)
	{
		serverSocket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
		if (serverSocket == -1)
			continue;

		// kernel has a wait period for ports to be reusable after a socket has been closed under normal behaviour
		// setsocketopt specifically the 3rd argument SO_REUSEADDR allows the address and port to be reusable
		// in our case we just prepare the socket to be able to bind to an address that already in use
		// that would be if the server crashed and restarted the previous address bound to the socket wouldn't be unavaible due to the kernel wait period 

		int yes = 1;
		if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
		{
			std::cerr << "[WEBSERV]\t>";
			perror("setsockopt");
			close(serverSocket);
			freeaddrinfo(res);
			exit(errno);
		}

		if (bind(serverSocket, it->ai_addr, it->ai_addrlen) == 0)
			break;
		close(serverSocket);
	}
	freeaddrinfo(res);
	if (!it)
	{
		close(serverSocket);
		std::cerr << "[WEBSERV]\t> Failed to Bind Any Socket..." << std::endl;
		exit(errno);
	}
	return (serverSocket);
}

void    Webserv::listenForConnections(int& serverSocket)
{
	if (listen(serverSocket, BACKLOG) == -1)
	{
		std::cerr << "[WEBSERV]\t>";
		perror("listen");
		close(serverSocket);
		exit(errno);
	}
	if (fcntl(serverSocket, F_SETFL, O_NONBLOCK, FD_CLOEXEC) == -1) // sets the socket to nonblock mode so it doesn't "block" on I/O operations (accept(), recv() ..)
	{
		std::cerr << "[WEBSERV]\t>";
		perror("fcntl");
		close(serverSocket);
		exit(errno);
	}
}

void	Webserv::initServers()
{
	int													serverSocket;
	std::vector<ServerConfig>::iterator					it;
	std::map<std::pair<std::string, std::string>, int>	boundServers;

	for (it = servers.begin(); it != servers.end(); it++)
	{
		std::pair<std::string, std::string> bindAddress = std::make_pair(it->host, it->port);
		std::map<std::pair<std::string, std::string>, int>::iterator deja = boundServers.find(bindAddress);

		// if server already bound just add the virtual server config to the ServerHandler
		if (deja != boundServers.end())
		{
			static_cast<ServerHandler *>(handlerMap[deja->second])->addVServer(*it);
			continue ;
		}

		// resolves domain name bind serverSocket to sockaddr and returns a valid socket
		serverSocket = bindSocket(it->host, it->port);
		listenForConnections(serverSocket);
		std::cout << "[WEBSERV]\t " << BLUE << BOLD << "Server listening on " << it->host << ":" << it->port << RESET << std::endl;

		ServerHandler	*handler = new ServerHandler(serverSocket);
		handler->addVServer(*it);
		boundServers.insert(std::make_pair(bindAddress, serverSocket));
		registerHandler(serverSocket, handler, EPOLLIN);
	}
}

void	Webserv::run()
{
	struct epoll_event events[MAX_EVENTS];
	while (true)
	{
		int eventCount = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

		std::cout << "EVENT_COUNT (" << eventCount << ") :";
		for (int i = 0; i < eventCount; i++)
		{
			print_epoll_events(events[i].events);
		}
		std::cout << std::endl;
		for (int i = 0; i < eventCount; i++)
		{
			EventHandler	*handler = static_cast<EventHandler *>(events[i].data.ptr);
			try
			{
				// print_epoll_events(events[i].events);
				if (events[i].events & EPOLLERR)
				{
					int fd = handler->getFd();
					std::cerr << "[WEBSERV][ERROR]\t CLIENT ON SOCKET " << fd << " IS UNREACHABLE" << std::endl;
					removeHandler(fd);
				}
				else
					handler->handleEvent(events[i].events);
				// else if (events[i].events & EPOLLHUP)
				// {
				// 	int fd = handler->getFd();
				// 	std::cerr << "[WEBSERV]\t Client Disconnected..." << fd << std::endl;
				// 	removeHandler(fd);
				// }
			}
			catch (FatalError& err)
			{
				std::cerr << "[WEBSERV][ERROR]\t" << err.what() << std::endl;
				removeHandler(handler->getFd()); // not complete clean up on client and CGI
			}
		}
		// std::vector<std::pair<EventHandler *, std::time_t>>::iterator it;
		// for (it = Timer.begin(); it != Timer.end(); it++)
		// {
		// 	time_t now = time(0);
		// 	if ((now - it->second) >= TIMEOUT)
		// 	{
		// 		removeHandler(it->first.getFd());
		// 		kill(it->first.getPid(), SIGKILL);
		// 	}
		// }

	}
}