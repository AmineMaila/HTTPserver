#ifndef SERVERHANDLER_HPP
# define SERVERHANDLER_HPP

#include "../IEventHandler.hpp"
#include "../Client/ClientHandler.hpp"
#include "Webserv.hpp"
#include <vector>

class ServerHandler : public EventHandler
{
public:
	~ServerHandler();
	ServerHandler();
	ServerHandler(int fd);

	void	addVServer(ServerConfig& server);

	// server handler only handles read events
	void	handleEvent(uint32_t events);

private:
	int							socket;
	std::vector<ServerConfig>	vServers;
};

#endif