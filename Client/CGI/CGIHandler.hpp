#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "../../IEventHandler.hpp"
# include "../Response/Response.hpp"

# define CGI_BUFFER_SIZE 4096


enum CGIState
{
	CHUNKED,
	LENGTH
};

class FatalError;

class CGIHandler : public EventHandler, public Response
{
public:
	virtual ~CGIHandler();
	CGIHandler(int& clientSocket, RequestData *data);

	int		setup();
	// void	readCgi();
	void	handleEvent(uint32_t events);
	



	// int		getFd() const;
	pid_t	getPid() const;
	int		getFd() const
	{
		return (outfd);
	}

	std::vector<std::string>	headersToEnv();
	void						buildEnv();

	void	processCGIHeaders(std::map<std::string, std::string>& headersMap);
	void	parseCGIHeaders();

	void	readChunked();
	void	readLength();
	int		feedCgi(const char *buf);

	int		respond();

private:
	// created here
	int				outfd; // file descriptor where cgi writes its output into
	int				infd; // file descriptor where cgi reads input from
	pid_t			pid;
	bool			parseBool;
	enum CGIState	CGIState;
	std::vector<std::string> envvars;
};

#endif