#include "CGIHandler.hpp"

CGIHandler::~CGIHandler() {}

CGIHandler::CGIHandler() {}

CGIHandler::CGIHandler(std::string& interpreter, std::string& ext, std::string& scriptName, std::string& pathInfo, std::string& queryString)
{
	this->interpreter = interpreter;
	this->ext = ext;
	this->scriptName = scriptName;
	this->pathInfo = pathInfo;
	this->queryString = queryString;
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
			perror("dup2");
			exit(errno);
		}
		close(pipe_fd[1]);

		if (chdir(path) == -1)
		{
			std::cerr << "[WEBSERV]\t";
			perror("chdir");
			exit(errno);
		}

		std::vector<std::string> envVars;
		
		envVars.push_back("REQUEST_METHOD=" + input.method);
		envVars.push_back("SCRIPT_NAME=" + scriptName);
		envVars.push_back("PATH_INFO=" + pathInfo);
		envVars.push_back("QUERYSTRING=" + queryString);
		envVars.push_back("SERVER_PROTOCOL=HTTP/1.1");
		// needs headers to be formatted as env vars;
		std::vector<char *>	env;

		for (std::vector<std::string>::iterator it = envVars.begin(); it != envVars.end(); it++)
		{
			env.push_back(const_cast<char *>(it->c_str()));
		}
		env.push_back(NULL);

		char *arg[3];
		arg[0] = const_cast<char *>(interpreter.c_str());
		arg[1] = const_cast<char *>(scriptName.c_str());
		arg[2] = NULL;
		if(execve(arg[0], arg, env.data()) == -1)
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
	std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++CGI READ" << std::endl;
	char	buffer[SEND_BUFFER_SIZE] = {0};
	int bytesRead = read(input.cgi.fd, buffer, SEND_BUFFER_SIZE);
	if (bytesRead > 0)
	{
		data = std::string(buffer, bytesRead);
		state = SENDDATA;
	}
	else if (bytesRead == 0)
	{
		state = FINISHED;
	}
	else
	{
		std::cerr << "[WEBSERV]\t";
		perror("read");
		state = ERROR;
	}
	HTTPserver->updateHandler(/*clientSocket*/, EPOLLOUT | EPOLLHUP);
	HTTPserver->updateHandler(fd, 0);
}