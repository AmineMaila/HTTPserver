#include "CGIHandler.hpp"

CGIHandler::~CGIHandler() {}

CGIHandler::CGIHandler(std::string& path, char **args, char **env)
{
	this->path = path;
	this->args = args;
	this->env = env;
}

int CGIHandler::getFd() const
{
	return (fd);
}

bool	CGIHandler::setup()
{
	int pipe_fd[2];

	if (pipe(pipe_fd) == -1)
	{
		std::cerr << "[WEBSERV]\t";
		perror("pipe");
		return (false);
	}

	int pid = fork();
	if (pid == -1)
	{
		std::cerr << "[WEBSERV]\t";
		perror("fork");
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		return (false);
	}
	else if (pid == 0)
	{
		close(pipe_fd[0]);
		if(dup2(pipe_fd[1], 1) == -1)
		{
			std::cerr << "[WEBSERV]\t";
			close(pipe_fd[1]);
			perror("dup2");
			exit(errno);
		}
		close(pipe_fd[1]);

		if (chdir(path.c_str()) == -1)
		{
			std::cerr << "[WEBSERV]\t";
			perror("chdir");
			exit(errno);
		}

		if(execve(args[0], args, env) == -1)
		{
			std::cerr << "[WEBSERV]\t";
			perror("execve");
			exit(errno);
		}
	}
	pid = pid;
	fd = pipe_fd[0];
	return (true);
}

void	CGIHandler::handleEvent(uint32_t events)
{
	(void)events;
	// std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++CGI READ" << std::endl;
	// char	buffer[SEND_BUFFER_SIZE] = {0};
	// int bytesRead = read(fd, buffer, SEND_BUFFER_SIZE);
	// if (bytesRead > 0)
	// {
	// 	client->setResponseBuffer(std::string(buffer, bytesRead));
	// 	data = std::string(buffer, bytesRead);
	// 	state = SENDDATA;
	// }
	// else if (bytesRead == 0)
	// {
	// 	state = FINISHED;
	// }
	// else
	// {
	// 	std::cerr << "[WEBSERV]\t";
	// 	perror("read");
	// 	state = ERROR;
	// }
	// HTTPserver->updateHandler(client->getSocket(), EPOLLOUT | EPOLLHUP);
	// HTTPserver->updateHandler(fd, 0);
}