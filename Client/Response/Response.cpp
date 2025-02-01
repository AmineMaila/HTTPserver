#include "Response.hpp"
#include "Error.hpp"

Response::~Response()
{
	if (dirList)
	{
		closedir(dirList);
		dirList = NULL;
	}
}

Response::Response() : contentLength(0), currRange(0), state(READBODY), nextState(READBODY)
{
	dirList = NULL;
	statusCodes.insert(std::make_pair(200, "OK"));
    statusCodes.insert(std::make_pair(201, "Created"));
    statusCodes.insert(std::make_pair(204, "No Content"));
    statusCodes.insert(std::make_pair(206, "Partial Content"));
    statusCodes.insert(std::make_pair(301, "Moved Permanently"));
    statusCodes.insert(std::make_pair(302, "Found"));
    statusCodes.insert(std::make_pair(303, "See Other"));
    statusCodes.insert(std::make_pair(307, "Temporary Redirect"));
    statusCodes.insert(std::make_pair(308, "Permanent Redirect"));

	// *****************MIME_TYPES****************** //


	mimeTypes.insert(std::make_pair(".html", "text/html"));
	mimeTypes.insert(std::make_pair(".htm", "text/html"));
	mimeTypes.insert(std::make_pair(".css", "text/css"));
	mimeTypes.insert(std::make_pair(".csv", "text/csv"));
	mimeTypes.insert(std::make_pair(".doc", "application/doc"));
	mimeTypes.insert(std::make_pair(".gif", "image/gif"));
	mimeTypes.insert(std::make_pair(".jpg", "image/jpeg"));
	mimeTypes.insert(std::make_pair(".jpeg", "image/jpeg"));
	mimeTypes.insert(std::make_pair(".js", "text/javascript"));
	mimeTypes.insert(std::make_pair(".json", "application/json"));
	mimeTypes.insert(std::make_pair(".jar", "application/java-archive"));
	mimeTypes.insert(std::make_pair(".mp3", "audio/mpeg"));
	mimeTypes.insert(std::make_pair(".mp4", "video/mp4"));
	mimeTypes.insert(std::make_pair(".mpeg", "video/mpeg"));
	mimeTypes.insert(std::make_pair(".png", "image/png"));
	mimeTypes.insert(std::make_pair(".pdf", "application/pdf"));
	mimeTypes.insert(std::make_pair(".sh", "application/x-sh"));
	mimeTypes.insert(std::make_pair(".wav", "audio/wav"));
	mimeTypes.insert(std::make_pair(".weba", "audio/webm"));
	mimeTypes.insert(std::make_pair(".webm", "video/webm"));
	mimeTypes.insert(std::make_pair(".webp", "image/webp"));
	mimeTypes.insert(std::make_pair(".xml", "application/xml"));
	mimeTypes.insert(std::make_pair(".zip", "application/zip"));
	mimeTypes.insert(std::make_pair(".tar", "application/x-tar"));
	mimeTypes.insert(std::make_pair(".bin", "application/octet-stream"));
	mimeTypes.insert(std::make_pair(".avif", "image/avif"));
	mimeTypes.insert(std::make_pair(".avi", "video/x-msvideo"));
}

Response::Response(const Response& rhs)
{
	*this = rhs;
}

Response&	Response::operator=(const Response& rhs)
{
	if (this != &rhs)
    {
        input = rhs.input;
		contentType = rhs.contentType;
		contentLength = rhs.contentLength;
		headers = rhs.headers;
		dirList = rhs.dirList;
		ranges = rhs.ranges;
		boundary = rhs.boundary;
		currRange = rhs.currRange;
		state = rhs.state;
		nextState = rhs.nextState;
		buffer = rhs.buffer;
		bodyFile.close();
		if (dirList)
		{
			closedir(dirList);
			dirList = NULL;
		}
	}
	return (*this);
}

void	Response::setInput(struct ResponseInput& input)
{
	this->input = input;
}

void	Response::setBuffer(const std::string& data)
{
	this->buffer = data;
}

int		Response::getStatusCode() const
{
	return (input.status);
}

bool	Response::sendData(int& socket)
{
	ssize_t bytesSent = send(socket, buffer.c_str(), buffer.length(), 0);
	// int err = errno;
	// std::cerr << bytesSent << std::endl;
	// errno = err;
	if (bytesSent == -1)
	{
		throw(FatalError(strerror(errno)));
	}
	std::cout << "--------RESPONSE_DATA_TO_CLIENT " << socket << "----" << input.path << "----" << std::endl;
	std::cout <<  "RESOURCE : "<< input.path << std::endl;
	std::cout << buffer << std::endl;
	std::cout << buffer.size() << std::endl;
	std::cout << "-----------------------------------------------------"  << std::endl;
	buffer.erase(0, bytesSent);
	return (buffer.empty());
}

void printState(enum State state, std::string name);

int	Response::sendResponse( int& socket )
{
	// printState(state, "State");
	// printState(nextState, "NextState");
	switch (state)
	{
		case READBODY:
			readBody();
			break;
		case READCHUNK:
			readChunk();
			break;
		case LISTDIR:
			directoryListing();
			break;
		case NEXTRANGE:
			getNextRange();
			break;
		case READRANGE:
			readRange();
			break;
		case SENDDATA:
			if(sendData(socket) == true)
				state = nextState;
			break;
		case FINISHED:
			return (1);
	}
	return (0);
}


/////////////////

void printState(enum State state, std::string name)
{
	switch (state)
	{
		case READBODY:
			std::cout << name << "==========>READING BODY" << std::endl;
			break;
		case READCHUNK:
			std::cout << name << "==========>READING CHUNK" << std::endl;
			break;
		case LISTDIR:
			std::cout << name << "==========>LISTING DIR" << std::endl;
			break;
		case NEXTRANGE:
			std::cout << name << "==========>GETTING NEXT RANGE" << std::endl;
			break;
		case READRANGE:
			std::cout << name << "==========>READING RANGE" << std::endl;
			break;
		case SENDDATA:
			std::cout << name << "==========>SENDING DATA" << std::endl;
			break;
		case FINISHED:
			std::cout << name << "==========>FINISHED" << std::endl;
			break;
	}
}