#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include "../IEventHandler.hpp"
#include "../Config/Config.hpp"
#include <algorithm>
#include <netdb.h>
#include <map>
#include <list>
#include <vector>
#include <ctime>
#include <sys/wait.h>

# define MAX_EVENTS 10
# define BACKLOG 128
# define TIMEOUT 5 // in seconds
# define CGI_TIMEOUT 15 // in seconds
# define EPOLL_TIMEOUT 1000 // in ms
# define PROCESS_LIMIT 25

class Webserv
{
public:
	~Webserv();
	Webserv(std::vector<ServerConfig>& servers);

	void	eraseDependency(EventHandler *dependent);
	void	registerDependency(EventHandler *dependent, EventHandler *dependency);
	void	registerHandler(const int fd, EventHandler *h, uint32_t events);
	void	updateHandler(const int fd, uint32_t events);
	void	removeHandler(const int fd);

	void	addTimer(EventHandler *client);
	void	updateTimer(EventHandler *client);
	void	eraseTimer(EventHandler *client);
	void	clientTimeout();
	void	incCgiCounter();
	void	decCgiCounter();
	int		getCgiCounter() const;

	void	collect(EventHandler *handler);
	void	cleanup(EventHandler *handler);

	bool		initServers();
	static void	stop();
	void		run();

private:
	int 	bindSocket(std::string& host, std::string& port);
	int		listenForConnections(int& listener);

	static bool						running;
	int								cgiCounter;

	int								epoll_fd;
	std::vector<ServerConfig>		servers;

	std::map<int, EventHandler *>				handlerMap;
	std::map<EventHandler *, EventHandler *>	dependencyMap;

	std::list<EventHandler *>					deleted;

	std::vector<std::pair<EventHandler *, time_t> >				clientTimer;
	std::vector<std::pair<EventHandler *, time_t> >::iterator	timeIt;
};

#endif