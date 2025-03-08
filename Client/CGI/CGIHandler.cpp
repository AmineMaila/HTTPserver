#include "CGIHandler.hpp"
#include "../../HTTPServer/Webserv.hpp"

CGIHandler::~CGIHandler()
{
	if (cgiSocket != -1)
		HTTPserver->removeHandler(cgiSocket);

	if (pid != -1 && waitpid(pid, NULL, WNOHANG) == 0)
		kill(pid, SIGTERM);
}

CGIHandler::CGIHandler(int& clientSocket, RequestData *data) : AResponse(clientSocket, data), cgiSocket(-1), inBodySize(0), pid(-1), headersParsed(false), chunked(false)
{
	execCGI();
}

int	CGIHandler::getFd() const
{
	return (cgiSocket);
}

pid_t CGIHandler::getPid() const
{
	return (pid);
}

bool	CGIHandler::storeBody()
{
	int		bytesWritten = write(cgiSocket, buffer.c_str(), buffer.size());
	if (bytesWritten == -1)
		throw(Disconnect("\tClient " + _toString(socket) + " : write: " + strerror(errno)));
	else if (bytesWritten == 0 && !buffer.empty())
		throw(Disconnect("\tClient " + _toString(socket) + " : write: unable to write"));
	buffer.erase(0, bytesWritten);
	return (buffer.empty());
}

void	CGIHandler::setBuffer(std::string buffer)
{
	if (!buffer.empty())
	{
		if (buffer.size() > reqCtx->contentLength)
			throw(Code(400));
		this->buffer = buffer;
		inBodySize += buffer.size();
		if (inBodySize > reqCtx->config->client_max_body_size)
			throw(Code(413));
		HTTPserver->updateHandler(socket, 0);
		HTTPserver->updateHandler(cgiSocket, EPOLLOUT);
	}
}

void	CGIHandler::setBuffer(char *buf, ssize_t size)
{
	buffer = std::string(buf, size);
	inBodySize += size;
	if (inBodySize > reqCtx->contentLength)
		throw(Code(400));
	else if (inBodySize > reqCtx->config->client_max_body_size)
		throw(Code(413));
	HTTPserver->updateHandler(socket, 0);
	HTTPserver->updateHandler(cgiSocket, EPOLLOUT);
}

void	CGIHandler::readCGIChunked()
{
	char	buf[SEND_BUFFER_SIZE] = {0};
	int		bytesRead = read(cgiSocket, buf, SEND_BUFFER_SIZE);
	if (bytesRead == -1)
		throw(Disconnect("\tClient " + _toString(socket) + " : read: " + strerror(errno)));

	buffer = buildChunk(buf, bytesRead);
	if (bytesRead == 0)
		nextState = DONE;
	if ((this->*sender)() == true)
		state = nextState;
	else
		state = WRITE;
}

void		CGIHandler::readCGILength()
{
	char	buf[SEND_BUFFER_SIZE] = {0};
	int		bytesRead = read(cgiSocket, buf, SEND_BUFFER_SIZE);
	if (bytesRead == -1)
	{
		throw(Disconnect("\tClient " + _toString(socket) + " : read: " + strerror(errno)));
	}
	else if (bytesRead > 0)
	{
		buffer.append(buf, bytesRead);
		if (headersParsed)
			state = WRITE;
		else
			state = HEADERS;
	}
	else
	{
		if (headersParsed)
			state = DONE;
		else
		{
			state = HEADERS;
			nextState = DONE;
		}
	}
}

int		CGIHandler::respond()
{
	switch (state)
	{
		case HEADERS:
			generateHeaders();
			break;
		case READ:
			HTTPserver->updateHandler(socket, 0);
			HTTPserver->updateHandler(cgiSocket, EPOLLIN);
			break;
		case WRITE:
			if ((this->*sender)() == true)
				state = nextState;
			break;
		case DONE:
			return (1);
	}
	return (0);
}

void	CGIHandler::handleEvent(uint32_t events)
{
	if (events & EPOLLIN)
	{
		if (chunked)
			readCGIChunked();
		else
			readCGILength();
		HTTPserver->updateHandler(socket, EPOLLOUT);
		HTTPserver->updateHandler(cgiSocket, 0);
	}
	else if (events& EPOLLOUT)
	{
		if (storeBody() == true)
		{
			if (inBodySize == reqCtx->contentLength)
			{
				HTTPserver->updateHandler(socket, 0);
				HTTPserver->updateHandler(cgiSocket, EPOLLIN);
			}
			else
			{
				HTTPserver->updateHandler(socket, EPOLLIN);
				HTTPserver->updateHandler(cgiSocket, 0);
			}
		}
	}
}