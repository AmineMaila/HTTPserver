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


	void	kickCGI(int code);

	void	reset();
	void 	handleRead();
	void 	handleWrite();
	void	handleEvent(uint32_t events);

	void	createResponse();
	void	deleteResponse();
	
	bool	childStatus();
	time_t	getCgiTimer() const;
	int		getFd() const;
	bool	getCgiActive() const;

private:
	int							socket;

	time_t						cgiTimer;
	bool						cgiActive;
	AResponse					*response;
	Request						request;

	std::vector<ServerConfig>&	vServers;
	Directives					config;
	
	e_reqType					reqState;
};

#endif