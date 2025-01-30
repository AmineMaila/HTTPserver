#include "Response.hpp"

Response::~Response()
{
	if (dirList)
	{
		closedir(dirList);
		dirList = NULL;
	}
}

Response::Response() : contentLength(0), chunked(false), currRange(0), state(BUILDHEADER), nextState(READBODY)
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
    statusCodes.insert(std::make_pair(400, "Bad Request"));
    statusCodes.insert(std::make_pair(403, "Forbidden"));
    statusCodes.insert(std::make_pair(404, "Not Found"));
    statusCodes.insert(std::make_pair(405, "Method Not Allowed"));
    statusCodes.insert(std::make_pair(413, "Payload Too Large"));
    statusCodes.insert(std::make_pair(415, "Unsupported Media Type"));
    statusCodes.insert(std::make_pair(416, "Range Not Satisfiable"));
    statusCodes.insert(std::make_pair(500, "Internal Server Error"));
    statusCodes.insert(std::make_pair(501, "Not Implemented"));
    statusCodes.insert(std::make_pair(504, "Gateway Timeout"));
    statusCodes.insert(std::make_pair(505, "HTTP Version Not Supported"));

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
		chunked = rhs.chunked;
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

void	Response::generateHeaders( void )
{
	headers.append("\r\nServer: webserv/1.0");
	headers.append("\r\nDate: " + getDate());

	if (input.status >= 400)
		generateErrorPage();
	else if (input.method == "GET") // check allowed methods
		handleGET();
	else if (input.method == "POST")
		handlePOST();
	else if (input.method == "DELETE")
		handleDELETE();

	if (input.requestHeaders.find("Connection") != input.requestHeaders.end())
		headers.append("\r\nConnection: " + input.requestHeaders["Connection"]);

	headers.append("\r\n\r\n");

	headers = ("HTTP/1.1 " + _toString(input.status) + " " + statusCodes[input.status]) + headers; // status line
	buffer.insert(0, headers);
	state = SENDDATA;
}

bool	Response::sendData(int& socket)
{
	// std::cout <<  "RESOURCE : "<< input.path << std::endl;
	// std::cout << "SENT DATA's SIZE : " << buffer.length() << std::endl;
	ssize_t bytesSent = send(socket, buffer.c_str(), buffer.length(), 0);
	// std::cout << "BYTESSENT: "  << bytesSent << std::endl;
	if (bytesSent == -1)
	{
		std::cerr << "[WEBSERV]\tsend: " << strerror(errno) << std::endl;
		state = ERROR;
		return (false);
	}
	std::cout << "--------RESPONSE_DATA_TO_CLIENT " << socket << "--------" << std::endl;
	std::cout << "\tSENT DATA OF SIZE: " << buffer.size() << std::endl;
	std::cout << "-----------------------------------------------------"  << std::endl;
	buffer.erase(0, bytesSent);
	return (buffer.empty());
}

void printState(enum State state, std::string name);

int	Response::sendResponse( int& socket )
{
	printState(state, "State");
	printState(nextState, "NextState");
	switch (state)
	{
		case BUILDHEADER:
			generateHeaders();
			break;
		case READBODY:
			readBody();
			break;
		case AUTOINDEX:
			autoIndex();
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
		case ERROR:
			return (-1);
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
		case BUILDHEADER:
			std::cout << name << "==========>BUILDING HEADER" << std::endl;
			break;
		case READBODY:
			std::cout << name << "==========>READING BODY" << std::endl;
			break;
		case AUTOINDEX:
			std::cout << name << "==========>AUTOINDEX" << std::endl;
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
		case ERROR:
			std::cout << name << "==========>ERROR" << std::endl;
			break;
		case FINISHED:
			std::cout << name << "==========>FINISHED" << std::endl;
			break;
	}
}