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
			std::cerr << "[WEBSERV][ERROR]\t";
			perror("accept");
			return;
		}

		// set non blocking mode and close the fd on execve
		if (fcntl(clientSocket, F_SETFL, FD_CLOEXEC) == -1)
		{
			std::cerr << "[WEBSERV][ERROR]\t" << std::endl;
			perror("fcntl");
			close(clientSocket);
			return;
		}

		ClientHandler	*client = new ClientHandler(clientSocket, this->vServers);
		HTTPserver->registerHandler(clientSocket, client, EPOLLIN);
		HTTPserver->addTimer(clientSocket);

		std::cout << "[SERVER]\tClient Connected To Socket " << clientSocket << "..." << std::endl;
	}
	else if (events & EPOLLHUP)
	{
		throw(Disconnect("[SERVER-" + _toString(socket) + "] SHUTDOWN"));
	}
}