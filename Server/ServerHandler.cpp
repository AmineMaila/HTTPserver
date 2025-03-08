#include "ServerHandler.hpp"

ServerHandler::~ServerHandler()
{
	HTTPserver->removeHandler(socket);
}

ServerHandler::ServerHandler() {}

ServerHandler::ServerHandler(int fd) : socket(fd) {}

int		ServerHandler::getFd() const
{
	return (socket);
}

void	ServerHandler::addVServer(ServerConfig& server)
{
	vServers.push_back(server);
}

void	ServerHandler::handleEvent(uint32_t events)
{
	if (events & EPOLLIN)
	{
		int	clientSocket = accept(socket, NULL, NULL);
		if (clientSocket == -1)
		{
			std::cerr << YELLOW << "\tWebserv : accept : " << strerror(errno) << RESET << std::endl;
			return;
		}

		if (fcntl(clientSocket, F_SETFD, FD_CLOEXEC) == -1)
		{
			std::cerr << YELLOW << "\tWebserv : fcntl : " << strerror(errno) << RESET << std::endl;
			close(clientSocket);
			return;
		}

		ClientHandler	*client = new (std::nothrow) ClientHandler(clientSocket, this->vServers);
		if (!client)
		{
			close(clientSocket);
			std::cerr << "\tServer " + _toString(socket) + " : Memory allocation failed" << std::endl;
			return;
		}
		HTTPserver->registerHandler(clientSocket, client, EPOLLIN);
		HTTPserver->addTimer(client);

		std::cout << CYAN << "\tClient Connected To Socket " << clientSocket << RESET << std::endl;
	}
	else if (events & EPOLLHUP)
	{
		throw(Disconnect("\tServer " + _toString(socket) + " : Shutdown"));
	}
}