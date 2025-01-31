#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "../../IEventHandler.hpp"

# define CGI_BUFFER_SIZE 4096

enum CGI_STATE
{
	CGI_BOOT,
	READ_CGI_CHUNK,
	SEND_CGI_CHUNK,
	CGI_ERROR,
	CGI_FINISHED,
};

class CGIHandler : public EventHandler
{
public:
	~CGIHandler();
	CGIHandler(int& clientSocket, std::string& path, char **args, char **env);

	int		setup();
	void	handleEvent(uint32_t events);

	int		getFd() const;
	pid_t	getPid() const;
	void	setQueryString(const std::string& other);
	void	setScriptName(const std::string& other);
	void	setPathInfo(const std::string& other);



private:
	// needed
	enum CGI_STATE	state;
	std::string		buffer;

	// created here
	int				fd; // file descriptor where cgi writes its output into
	pid_t			pid;

	// input
	std::string		path;
	char			**env;
	char			**args;
	int				clientSocket;
};

#endif