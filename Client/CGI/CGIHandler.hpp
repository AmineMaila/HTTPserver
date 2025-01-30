#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

#include "../../IEventHandler.hpp"

class CGIHandler : public EventHandler
{
public:
	~CGIHandler();
	CGIHandler(int& fd, std::string& path, char **args, char **env);

	bool	setup();
	void	handleEvent(uint32_t events);

	int		getFd() const;
	void	setQueryString(const std::string& other);
	void	setScriptName(const std::string& other);
	void	setPathInfo(const std::string& other);



private:
	ClientHandler	*client;
	int				fd; // file descriptor where cgi writes its output into
	pid_t			pid;
	std::string		path;
	char			**env;
	char			**args;
};

#endif