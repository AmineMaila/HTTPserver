#ifndef IEVENTHANDLER_HPP
# define IEVENTHANDLER_HPP

# include <sys/epoll.h>
# include <sys/socket.h>
# include <fcntl.h>
# include <unistd.h>
# include <iostream>
# include "Webserv.hpp"

class EventHandler
{
public:
	virtual ~EventHandler() {}
	virtual	void	handleEvent(uint32_t events) = 0;
	Webserv	*HTTPserver;
};

#endif