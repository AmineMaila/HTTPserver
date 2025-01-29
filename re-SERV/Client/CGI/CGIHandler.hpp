#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

#include "../../IEventHandler.hpp"

class CGIHandler : public EventHandler
{
public:
	~CGIHandler();
	CGIHandler();
	CGIHandler(std::string& interpreter, std::string& ext, std::string& scriptName, std::string& pathInfo, std::string& queryString);

	bool	setup();
	void	handleEvent(uint32_t events);

	int		getFd() const;
	void	setQueryString(const std::string& other);
	void	setScriptName(const std::string& other);
	void	setPathInfo(const std::string& other);



private:
	pid_t		pid;
	int			fd; // file descriptor where cgi writes its output into
	std::string	interpreter; // path to the cgi interpreter ex. /usr/bin/python3 for .py
	std::string	ext; // cgi script extention
	std::string	scriptName;
	std::string	pathInfo;
	std::string	queryString;
};

#endif