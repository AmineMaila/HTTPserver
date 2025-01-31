#include "Webserv.hpp"
#include "../Server/ServerHandler.hpp"

Webserv::~Webserv() {}

Webserv::Webserv()
{
	epoll_fd = epoll_create1(0);
}

Webserv::Webserv(std::vector<ServerConfig>& servers) : servers(servers) {}

void	Webserv::registerHandler(int fd, EventHandler *h, uint32_t events)
{
	struct epoll_event ev;
	ev.events = events | EPOLLET;
	ev.data.ptr = h;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
	handlerMap[fd] = h;
	h->HTTPserver = this;
}

void	Webserv::updateHandler(const int fd, uint32_t events)
{
	struct epoll_event ev;

	ev.events = events | EPOLLET;
	ev.data.ptr = handlerMap[fd];
	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}

void	Webserv::removeHandler(int fd)
{
	std::map<int, EventHandler*>::iterator it = handlerMap.find(fd);
	if (it != handlerMap.end())
	{
		EventHandler	*h = it->second;
		handlerMap.erase(it);
		delete h;
	}
	close(fd);
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}


int	Webserv::bindSocket(std::string& host, std::string& port)
{
	struct addrinfo	hints;
	
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
	int listener;
	struct addrinfo	*p;

	for (p = res; p; p = p->ai_next)
	{
		listener = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
		if (listener == -1)
			continue;

		// kernel has a wait period for ports to be reusable after a socket has been closed under normal behaviour
		// setsocketopt specifically the 3rd argument SO_REUSEADDR allows the address and port to be reusable
		// in our case we just prepare the socket to be able to bind to an address that already in use
		// that would be if the server crashed and restarted the previous address bound to the socket wouldn't be unavaible due to the kernel wait period 

		int yes = 1;
		if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
		{
			std::cerr << "[WEBSERV]\t>";
			perror("setsockopt");
			close(listener);
			freeaddrinfo(res);
			exit(errno);
		}

		if (bind(listener, p->ai_addr, p->ai_addrlen) == 0)
			break;
		close(listener);
	}
	freeaddrinfo(res);
	if (!p)
	{
		close(listener);
		std::cerr << "[WEBSERV]\t> Could Not Bind Any Socket..." << std::endl;
		exit(errno);
	}
	return (listener);
}

void    Webserv::listenForConnections(int& listener)
{
	if(listen(listener, BACKLOG) == -1)
	{
		std::cerr << "[WEBSERV]\t>";
		perror("listen");
		exit(errno);
	}
	if (fcntl(listener, F_SETFL, O_NONBLOCK, FD_CLOEXEC) == -1) // sets the socket to nonblock mode so it doesn't "block" on I/O operations (accept(), recv() ..)
	{
		std::cerr << "[WEBSERV]\t> fcntl: " << strerror(errno) << std::endl;
		close(listener);
		exit(errno);
	}
}

void	Webserv::initServers()
{
	int serverSocket;
	std::vector<ServerConfig>::iterator it;
	std::map<std::pair<std::string, std::string>, int>	boundServers;

	for (it = servers.begin(); it != servers.end(); it++)
	{
		std::pair<std::string, std::string> bindAddress = std::make_pair(it->host, it->port);
		std::map<std::pair<std::string, std::string>, int>::iterator deja = boundServers.find(bindAddress);

		// if server already bound just add the virtual server config to the ServerHandler that was already bound
		if (deja != boundServers.end())
		{
			static_cast<ServerHandler *>(handlerMap[deja->second])->addVServer(*it);
			continue ;
		}

		serverSocket = bindSocket(it->host, it->port);
		listenForConnections(serverSocket);
		std::cout << "[WEBSERV]\t> Server listening on " << it->host << ":" << it->port << std::endl;

		ServerHandler	*h = new ServerHandler(serverSocket);
		h->addVServer(*it);
		boundServers.insert(std::make_pair(bindAddress, serverSocket));
		registerHandler(serverSocket, h, EPOLLIN);
	}
}

void	Webserv::run()
{
	struct epoll_event events[MAX_EVENTS];
	while (true)
	{
		int eventCount = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

		for (int i = 0; i < eventCount; i++)
		{
			EventHandler	*h = static_cast<EventHandler *>(events[i].data.ptr);
			h->handleEvent(events[i].events);
		}
		for (std::map<pid_t, std::time_t>::iterator it = CGITimer.begin(); it != CGITimer.end(); it++)
		{
			time_t now = time(0);
			if ((now - it->second) >= TIMEOUT)
			{
				kill(it->first, SIGKILL);
			}
		}
	}
}