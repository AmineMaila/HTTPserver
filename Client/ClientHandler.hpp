#ifndef CLIENTHANDLER_HPP
# define CLIENTHANDLER_HPP

# include "../IEventHandler.hpp"
# include "Response/Response.hpp"
# include "Request/Request.hpp"
# include "iostream"

class ClientHandler : public EventHandler
{
public:
	~ClientHandler();
	ClientHandler(int fd, std::vector<ServerConfig> vServers);


	ServerConfig&	matchingServer(std::string& host);
	void			decodeUri(struct ResponseInput& input, std::string& URL);
	void			initResponse();
	int				getSocket( void ) const;
	// void			setResponseBuffer(std::string buffer);


	void	reset();
	void 	handleEvent(uint32_t events);


private:
	int							socket;
	Request						request;
	Response					response;
	std::vector<ServerConfig>	vServers;
	int							cgifd;
	
};

#endif