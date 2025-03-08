# include "ClientHandler.hpp"
# include "Response/Error.hpp"
# include "CGI/CGIHandler.hpp"
# include "../HTTPServer/Webserv.hpp"

ClientHandler::~ClientHandler()
{
	HTTPserver->removeHandler(socket);
	HTTPserver->eraseTimer(this);
	deleteResponse();
}

ClientHandler::ClientHandler(int fd, std::vector<ServerConfig>& vServers) : request(vServers), vServers(vServers)
{
	socket = fd;
	cgiActive = false;
	cgiTimer= 0;
	response = NULL;
	reqState = REGULAR;
}

void	ClientHandler::reset()
{
	reqState = REGULAR;
	deleteResponse();
	request = Request(vServers);
}

time_t	ClientHandler::getCgiTimer() const { return (cgiTimer); }

bool	ClientHandler::getCgiActive() const { return (cgiActive); }

int	ClientHandler::getFd() const { return (socket); }

bool	ClientHandler::childStatus()
{
	if (!cgiActive)
		return (false);

	CGIHandler *cgi = static_cast<CGIHandler *>(response);
	int status;
	int ret = waitpid(cgi->getPid(), &status, WNOHANG);
	if (ret > 0)
	{
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		{
			kickCGI(500);
			return (true);
		}
	}
	return (false);
}

void	ClientHandler::kickCGI(int code)
{
	deleteResponse();
	response = new (std::nothrow) ErrorPage(Code(code), socket, request.getRequestData());
	if (!response)
		throw(Disconnect("\tClient " + _toString(socket) + " : Memory allocation failed"));
	HTTPserver->updateHandler(socket, EPOLLOUT);
}

void	ClientHandler::deleteResponse()
{
	if (response)
	{
		if (cgiActive)
		{
			CGIHandler *cgi = static_cast<CGIHandler *>(response);
			HTTPserver->eraseDependency(cgi);
			HTTPserver->collect(cgi);
			HTTPserver->decCgiCounter();
			cgiActive = false;
		}
		delete response;
		response = NULL;
	}
}

void	ClientHandler::createResponse()
{
	if (request.getRequestData()->isCGI)
	{
		if (HTTPserver->getCgiCounter() >= PROCESS_LIMIT)
			throw(Code(503));
		HTTPserver->updateHandler(socket, 0);
		CGIHandler	*cgi = new (std::nothrow) CGIHandler(socket, request.getRequestData());
		if (!cgi)
			throw(Disconnect("\tClient " + _toString(socket) + " : Memory allocation failed"));
		cgiTimer = time(NULL);
		cgiActive = true;
		HTTPserver->registerDependency(cgi, this);
		HTTPserver->registerHandler(cgi->getFd(), cgi, EPOLLIN);
		response = cgi;
		HTTPserver->incCgiCounter();
	}
	else
	{
		response = new (std::nothrow) Response(socket, request.getRequestData());
		if (!response)
			throw(Disconnect("\tClient " + _toString(socket) + " : Memory allocation failed"));
		HTTPserver->updateHandler(socket, EPOLLOUT);
	}
}

void 	ClientHandler::handleRead()
{
	char	buf[RECV_BUFFER_SIZE] = {0};

	ssize_t	bytesReceived = recv(socket, buf, RECV_BUFFER_SIZE, 0);
	if (bytesReceived == 0)
		throw(Disconnect("\tClient " + _toString(socket) + " : Closed Connection"));
	else if (bytesReceived < 0)
		throw(Disconnect("\tClient " + _toString(socket) + " : recv: " + strerror(errno)));
	else
	{
		int returnValue;
		switch (reqState)
		{
			case REGULAR:
				returnValue = request.process(buf, bytesReceived);
				if (returnValue == FORWARD_CGI)
				{
					if (HTTPserver->getCgiCounter() >= PROCESS_LIMIT)
						throw(Code(503));
					CGIHandler	*cgi = new (std::nothrow) CGIHandler(socket, request.getRequestData());
					if (!cgi)
						throw(Disconnect("\tClient " + _toString(socket) + " : Memory allocation failed"));
					cgiTimer = time(NULL);
					cgiActive = true;
					HTTPserver->registerDependency(cgi, this);
					HTTPserver->registerHandler(cgi->getFd(), cgi, 0);
					cgi->setBuffer(request.getBuffer());
					response = cgi;
					reqState = CGI;
					HTTPserver->incCgiCounter();
				}
				else if (returnValue == RESPOND)
					createResponse();
				break;
			case CGI:
				static_cast<CGIHandler *>(response)->setBuffer(buf, bytesReceived);
				break;
		}
	}
}

void 	ClientHandler::handleWrite()
{
	if (response->respond() == 1)
	{
		std::cout << MAGENTA
					<< "\t" << request.getRequestData()->Method
					<< " " << request.getRequestData()->URI
					<< " " << request.getRequestData()->statusCode
					<< RESET << std::endl;
		if (request.getRequestData()->keepAlive)
		{
			HTTPserver->updateTimer(this);
			HTTPserver->updateHandler(socket, EPOLLIN);
			this->reset();
		}
		else
			throw(Disconnect());
	}
}

void	ClientHandler::handleEvent(uint32_t events)
{
	HTTPserver->updateTimer(this);
	try
	{
		if (events & EPOLLIN)
		{
			handleRead();
		}
		else if (events & EPOLLOUT)
		{
			try
			{
				handleWrite();
			}
			catch (CGIRedirect& redirect)
			{
				deleteResponse();
				request.setFullPath(redirect.location);
				resolveAbsPath(*request.getRequestData());
				createResponse();
			}
		}
		else if (events & EPOLLHUP)
			throw(Disconnect("\tClient " + _toString(socket) + " : Closed connection"));
	}
	catch (Code& e)
	{
		deleteResponse();
		response = new (std::nothrow) ErrorPage(e, socket, request.getRequestData());
		if (!response)
			throw(Disconnect("\tClient " + _toString(socket) + " : Memory allocation failed"));
		HTTPserver->updateHandler(socket, EPOLLOUT);
	}
}
