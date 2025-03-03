#ifndef CLIENTHANDLER_HPP
# define CLIENTHANDLER_HPP

# include "../IEventHandler.hpp"
# include "Response/Response.hpp"
# include "Request/Request.hpp"
# include <iostream>

class ClientHandler : public EventHandler
{
public:
	~ClientHandler();
	ClientHandler(int fd, std::vector<ServerConfig>& vServers);


	void	reset();
	void 	handleRead();
	void 	handleWrite();
	void	handleEvent(uint32_t events);

	void	createResponse();
	void	deleteResponse();
	
	void	ctime();
	time_t	getElapsedTime() const;

	int		getFd() const;

private:
	time_t						elapsedTime;

	int							socket;
	Request						request;

	AResponse					*response;

	std::vector<ServerConfig>&	vServers;
	Directives					config;
	
	e_reqType					reqState;
};

#endif