#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "../../IEventHandler.hpp"
# include "../Response/AResponse.hpp"

class CGIHandler : public EventHandler, public AResponse
{
public:
	virtual ~CGIHandler();
	CGIHandler(int& clientSocket, RequestData *data);
	CGIHandler(const CGIHandler& rhs);
	CGIHandler& operator=(const CGIHandler& rhs);
	
	pid_t	getPid() const;
	int		getFd() const;

	void	buildEnv();


	void	execCGI();
	void	handleEvent(uint32_t events);

	void	validateHeaders();
	void	parseHeaders();
	void	addHeaders();
	void	generateHeaders();

	
	void	readCGILength();
	void	readCGIChunked();
	bool	storeBody();

	void	setBuffer(std::string buffer);
	void	setBuffer(char *buf, ssize_t size);


	int		respond();

private:
	// created here
	std::vector<std::string>			envVars;
	std::vector<char *>					envPtr;
	char								*args[3];
	std::map<std::string, std::string>	headersMap;

	int		cgiSocket;
	size_t	inBodySize;
	pid_t	pid;
	bool	headersParsed;
	void	(CGIHandler::*CGIreader)();
};

#endif