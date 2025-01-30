#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include "../IEventHandler.hpp"
#include "../Server/ServerHandler.hpp"
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

typedef struct												Directives {
	std::vector<std::pair<int, std::string> >				error_pages;
	int														client_max_body_size;
	std::string												root;
	std::string												alias;
	std::vector<std::string>								index;
	std::vector<std::string> 								methods;
	std::string 											upload_store;
	bool													autoindex;
	std::vector<std::string>								redirect; // std::pair<int, std::string>
	std::map<std::string, std::string>						cgi_ext; // std::map<std::string, std::string>
}															Directives;

typedef struct												ServerConfig {
	std::string												host;
	std::string												port;
	std::vector<std::string>								server_names;
	Directives												ServerDirectives;
	std::map<std::string, Directives>						Locations;
}															ServerConfig;

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