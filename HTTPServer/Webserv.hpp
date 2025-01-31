#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include "../IEventHandler.hpp"
#include "../Config/Config.hpp"
#include <algorithm>
#include <netdb.h>
#include <map>
#include <errno.h>
#include <vector>
#include <ctime>
#include <signal.h>
#include <cstring>

# define MAX_EVENTS 100
# define BACKLOG 128
# define TIMEOUT 60

class Webserv
{
public:
	~Webserv();
	Webserv();
	Webserv(std::vector<ServerConfig>& servers);

	void	registerHandler(const int fd, EventHandler *h, uint32_t events);
	void	updateHandler(const int fd, uint32_t events);
	void	removeHandler(const int fd);



	void	initServers();
	void	run();

private:
	int 	bindSocket(std::string& host, std::string& port);
	void    listenForConnections(int& listener);


	int								epoll_fd;
	std::vector<ServerConfig>		servers;
	std::map<int, EventHandler*>	handlerMap;
	std::map<pid_t, std::time_t>	CGITimer;
	// std::vector<ServerConfig>		vServers; // virtual servers from config file
};

#endif