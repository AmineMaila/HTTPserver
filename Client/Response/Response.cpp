#include "Response.hpp"

Response::~Response()
{
	if (dirList)
		closedir(dirList);
}

Response::Response(int &clientSocket, RequestData *data) : AResponse(clientSocket, data)
{
	dirList = NULL;
	reader = &Response::readBody;
}

Response& Response::operator=(const Response& rhs)
{
	if (this != &rhs)
	{
		socket = rhs.socket;
		state = rhs.state;
		nextState = rhs.nextState;
		headers = rhs.headers;
		buffer = rhs.buffer;
		sender = rhs.sender;
		reqCtx = rhs.reqCtx;
		rangeData = rhs.rangeData;
		dirList = rhs.dirList;
		contentType = rhs.contentType;
		contentLength = rhs.contentLength;
	}
	return (*this);
}

int	Response::respond()
{
	switch (state)
	{
		case HEADERS:
			generateHeaders();
			break;
		case READ:
			(this->*reader)();
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