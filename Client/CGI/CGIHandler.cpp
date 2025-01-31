#include "CGIHandler.hpp"
#include "../../HTTPServer/Webserv.hpp"

CGIHandler::~CGIHandler() {}

CGIHandler::CGIHandler(int& clientSocket, std::string& path, char **args, char **env) : clientSocket(clientSocket)
{
	this->path = path;
	this->args = args;
	this->env = env;
}

int CGIHandler::getFd() const
{
	return (fd);
}

pid_t CGIHandler::getPid() const
{
	return (pid);
}

int	CGIHandler::setup()
{
	int pipe_fd[2];

	if (pipe(pipe_fd) == -1)
	{
		std::cerr << "[WEBSERV]\t";
		perror("pipe");
		return (-1);
	}

	int pid = fork();
	if (pid == -1)
	{
		std::cerr << "[WEBSERV]\t";
		perror("fork");
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		return (-1);
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
	state = READ_CGI_CHUNK;
	return (fd);
}

void	CGIHandler::readCgi()
{
	// std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++CGI READ" << std::endl;
	char	buf[CGI_BUFFER_SIZE] = {0};
	int		bytesRead = read(clientSocket, buf, CGI_BUFFER_SIZE);
	if (bytesRead > 0)
	{
		// client->setResponseBuffer(std::string(buffer, bytesRead));
		buffer = std::string(buf, bytesRead);
		state = FORWARD_TO_RESPONSE;
	}
	else if (bytesRead == 0)
	{
		state = CGI_FINISHED;
	}
	else
	{
		std::cerr << "[WEBSERV]\t";
		perror("read");
		state = CGI_ERROR;
	}
	HTTPserver->updateHandler(clientSocket, EPOLLOUT | EPOLLHUP);
	HTTPserver->updateHandler(fd, 0);
}

void	CGIHandler::handleEvent(uint32_t events)
{
	if (events & EPOLLIN)
	{
		switch (state)
		{
		case READ_CGI_CHUNK:
			readCgi();
			break;
		case FORWARD_TO_RESPONSE:
			break;
		case CGI_FINISHED:
			break;
		case CGI_ERROR:
			break;
		}
	}
}