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
		data = rhs.data;
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

void	Response::handleDELETE( void )
{
	if (input.isDir)
	{
		if (rmdir(input.absolutePath.c_str()) == -1)
		{
			input.status = 500; // wrong
			generateErrorPage();
			return ;
		}
	}
	else 
	{
		if (remove(input.absolutePath.c_str()) == -1)
		{
			input.status = 500; // wrong
			generateErrorPage();
			return ;
		}
	}
	nextState = FINISHED;
}

void	Response::generateErrorPage( void )
{
	 // if (error_page directive exists)
	 	// open the error page in bodyFD
	 //else
		data = "<!DOCTYPE html>\n"
				"<html>\n"
				"<head>\n"
				"    <title> " + _toString(input.status) + " " + statusCodes[input.status] + " </title>\n"
				"</head>\n"
				"<body>\n"
				"    <center>\n"
				"        <h1> " + _toString(input.status) + " " + statusCodes[input.status] + " </h1>\n"
				"    </center>\n"
				"    <hr>\n"
				"    <center>webserv/1.0</center>\n"
				"</body>\n"
				"</html>";
	headers.append("\r\nContent-Type: text/html");
	headers.append("\r\nContent-Length: " + _toString(data.size()));
	nextState = ERROR;
}

void	Response::handlePOST( void )
{
	data = "<!DOCTYPE html>\n"
			"<html lang=\"en\">\n"
			"<head>\n"
			"    <meta charset=\"UTF-8\">\n"
			"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
			"    <title>Success Response</title>\n"
			"    <style>\n"
			"        body {\n"
			"            font-family: Arial, sans-serif;\n"
			"            margin: 40px auto;\n"
			"            max-width: 650px;\n"
			"            padding: 20px;\n"
			"        }\n"
			"        h1 {\n"
			"            color: #2c662d;\n"
			"        }\n"
			"        p {\n"
			"            color: #333;\n"
			"            line-height: 1.4;\n"
			"        }\n"
			"    </style>\n"
			"</head>\n"
			"<body>\n"
			"    <h1>Success</h1>\n"
			"    <p>Your request has been processed successfully.</p>\n"
			"    <p>The data has been saved to our system.</p>\n"
			"</body>\n"
			"</html>";
	headers.append("\r\nContent-Type: text/html");
	headers.append("\r\nContent-Length: " + _toString(data.size()));
	nextState = FINISHED;
}

bool	Response::getResource( void )
{
	// if (input.isDir)
	// {
	// 	std::vector<std::string>::iterator	it;

	// 	for (it = input.config.index.begin(); it != input.config.index.end(); it++)
	// 	{
	// 		if (access((input.absolutePath + *it).c_str(), F_OK) == 0) // file exists
	// 			break;
	// 	}
	// 	if (it == input.config.index.end())
	// 	{
	// 		if (input.config.autoindex)
	// 			return (true);
	// 		input.status = 404;
	// 		return (false);
	// 	}
	// 	input.absolutePath.append(*it);
	// 	input.isDir = false;
	// }
	// if (access(input.absolutePath.c_str(), R_OK) != 0) // read permission for files
	// {
	// 	input.status = 403;
	// 	return (false);
	// }

	if (input.cgi.isCgi)
	{
		execCGI();
	}
	else
	{
		bodyFile.open(input.absolutePath);
		if (!bodyFile.is_open())
		{
			input.status = 500;
			return (false);
		}

		contentType = getContentType(input.absolutePath, mimeTypes);
		contentLength = fileLength(input.absolutePath);
	}
	return (true);
}

bool	Response::parseRangeHeader( void ) // example => Range: bytes=0-499,1000-1499
{
	std::string	value = input.requestHeaders["Range"];
	std::string prefix = "bytes=";
	size_t pos = value.find(prefix);
	if (pos == std::string::npos)
		return (false);
	pos += prefix.length();


	std::istringstream	rangess(value.substr(pos));
	std::string			rangeStr;

	boundary = generateRandomString();

	while (std::getline(rangess, rangeStr, ','))
	{
		size_t	delim;
		char	*stop;

		delim = rangeStr.find("-");
		if (delim == std::string::npos)
			return (false);

		std::string startStr = rangeStr.substr(0, delim);
		if (startStr.empty())
			return (false);
		unsigned long start = std::strtoul(startStr.c_str(), &stop, 10);
		if (errno == ERANGE || !std::isdigit(*stop))
			return (false);

		std::string endStr = rangeStr.substr(delim + 1);
		if (endStr.empty())
			return (false);
		unsigned long end = std::strtoul(endStr.c_str(), &stop, 10);
		if (errno == ERANGE || !std::isdigit(*stop))
			return (false);

		if (start > end || end >= contentLength)
		{
			input.status = 416;
			generateErrorPage();
			return (false);
		}
	
		Range unit;

		unit.range = std::make_pair(start, end);
		unit.rangeLength = end - start + 1;
	
		unit.header.append("\r\n--" + boundary);
		unit.header.append("\r\nContent-Range: bytes " + startStr + "-" + endStr + "/" + _toString(contentLength));
		unit.header.append("\r\nContent-Type: " + contentType);
		unit.header.append("\r\n\r\n");

		ranges.push_back(unit);
	}
	ranges[0].header.erase(0, 2); // erases the first "\r\n"
	return (true);
}

int	Response::rangeContentLength( void )
{
	int ret = 0;
	std::vector<Range>::iterator it = ranges.begin();

	for (; it != ranges.end(); it++)
	{
		ret += it->header.length();
		ret += it->rangeLength;
	}
	ret += 8 + boundary.length(); // 8 is  the length of the constant "\r\n--" "--\r\n"
	return (ret);
}

void	Response::buildRange( void )
{
	if (!parseRangeHeader())
		return ;
	
	input.status = 206;
	if (ranges.size() == 1)
	{
		ranges[0].header.clear();
		ranges[0].headerSent = true;
		contentLength = ranges[0].rangeLength;
		headers.append("\r\nContent-Range: bytes " + _toString(ranges[0].range.first) + "-" + _toString(ranges[0].range.second) + "/" + _toString(fileLength(input.absolutePath)));
	}
	else
	{
		contentType = "multipart/byteranges; boundary=" + boundary;
		contentLength = rangeContentLength();
	}
	nextState = NEXTRANGE;
}

void	Response::handleGET( void )
{
	if (!getResource())
	{
		generateErrorPage();
		return ;
	}

	if (input.config.autoindex && input.isDir)
	{
		dirList = opendir(input.absolutePath.c_str()); // CHECK LEAK
		if (dirList == NULL)
		{
			std::cerr << "[WEBSERV]\t>";
			perror("opendir");
			input.status = 500;
			generateErrorPage();
			return ;
		}
		headers.append("\r\nTransfer-Encoding: chunked");
		contentType = "text/html";
		nextState = AUTOINDEX;
	}
	else
	{
		if (input.requestHeaders.find("Range") != input.requestHeaders.end())
			buildRange();
		if (contentType.find("video") != std::string::npos || contentType.find("video") != std::string::npos)
		{
			headers.append("\r\nTransfer-Encoding: chunked");
			chunked = true;
		}
		else
			headers.append("\r\nContent-Length: " + _toString(contentLength));
	}
	headers.append("\r\nContent-Type: " + contentType);
	headers.append("\r\nAccept-Ranges: bytes");
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
	data.insert(0, headers);
	state = SENDDATA;
}

void	Response::buildChunk()
{
	data = toHex(data.size()) + "\r\n" + data + "\r\n";
	if (nextState == FINISHED)
		data.append("0\r\n\r\n");
	state = SENDDATA;
}

void	Response::autoIndex()
{
	data.append("<html>\n"
				"<head>\n"
				"<title>Index of " + input.uri + "</title>\n"
				"</head>\n"
				"<body>\n"
				"<h1>Index of " + input.uri + "</h1>\n"
				"<hr>\n"
				"<pre>\n");
	nextState = LISTDIR;
	directoryListing();
}

void	Response::directoryListing()
{
	struct dirent	*entry;
	int 			i = 0;

	while (i < 100 && (entry = readdir(dirList)) != NULL)
	{
		std::string name = entry->d_name;
		if (name == ".")
			continue ;
		if (entry->d_type == DT_DIR)
			name.append("/");
		data.append("<a href=\"" + name + "\">" + name + "</a>\n");
		i++;
	}
	if (entry == NULL)
	{
		data.append("</pre>\n"
					"<hr>\n"
					"</body>\n"
					"</html>");
		nextState = FINISHED;
	}
	buildChunk();
}

void	Response::getNextRange()
{
	if (currRange == ranges.size())
	{
		if (ranges.size() > 1)
		{
			data = "\r\n--" + boundary + "--\r\n";
			state = SENDDATA;
			nextState = FINISHED;
		}
		else
			state = FINISHED;
	}
	else
	{
		std::cout << "RANGE LENGTH OF INDEX " << currRange << ": " << ranges[currRange].rangeLength << std::endl;
		data = ranges[currRange].header;
		bodyFile.seekg(ranges[currRange].range.first, std::ios::beg);
		state = READRANGE;
	}
}

void	Response::readRange()
{
	char buffer[SEND_BUFFER_SIZE] = {0};

	size_t readLength = std::min
	(
		static_cast<size_t>(SEND_BUFFER_SIZE),
		ranges[currRange].rangeLength
	);
	std::cout << "READ LENGTH OF RANGE :" << readLength << std::endl;
	int bytesRead = bodyFile.read(buffer, readLength).gcount();
	if (bytesRead == -1)
	{
		std::cerr << "[WEBSERV]\tread: " << strerror(errno) << std::endl; // errno after I/O forbidden
		state = ERROR;
	}
	else if (bytesRead > 0)
	{
		data += std::string(buffer, bytesRead);
		std::cout << "BUFFER SIZE IN RANGE : " << data.size() << std::endl;
		ranges[currRange].rangeLength -= bytesRead;
		state = SENDDATA;
		if (ranges[currRange].rangeLength == 0)
		{
			nextState = NEXTRANGE;
			currRange++;
		}
		else
			nextState = READRANGE;
	}
}

void	Response::readBody()
{
	char buffer[SEND_BUFFER_SIZE] = {0};
	int bytesRead = bodyFile.read(buffer, SEND_BUFFER_SIZE).gcount();
	if (bytesRead > 0)
	{
		// std::cout << "BYTESREAD : "<< bytesRead << std::endl;
		// buffer[bytesRead] = '\0'; // be CAREFUL
		data = std::string(buffer, bytesRead);
		// std::cout << "DATA SIZE AFTER READ: " << data.size() << std::endl;
		state = SENDDATA;
		if (chunked)
		{
			buildChunk();
			if (bodyFile.peek() == EOF)
				nextState = FINISHED;
		}
	}
	else if (bytesRead == 0)
	{
		state = FINISHED;
	}
	else
	{
		std::cerr << "[WEBSERV]\tread: " << strerror(errno) << std::endl;
		state = ERROR;
	}
}

void	Response::readCGI( int& socket )
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
}

bool	Response::sendData(int& socket)
{
	// std::cout <<  "RESOURCE : "<< input.absolutePath << std::endl;
	// std::cout << "SENT DATA's SIZE : " << data.length() << std::endl;
	ssize_t bytesSent = send(socket, data.c_str(), data.length(), 0);
	// std::cout << "BYTESSENT: "  << bytesSent << std::endl;
	if (bytesSent == -1)
	{
		std::cerr << "[WEBSERV]\tsend: " << strerror(errno) << std::endl;
		state = ERROR;
		return (false);
	}
	std::cout << "--------RESPONSE_DATA_TO_CLIENT " << socket << "--------" << std::endl;
	std::cout << "\tSENT DATA OF SIZE: " << data.size() << std::endl;
	std::cout << "-----------------------------------------------------"  << std::endl;
	data.erase(0, bytesSent);
	return (data.empty());
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
		case READCGI:
			readCGI( socket );
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
		case READCGI:
			std::cout << name << "==========>READING CGI" << std::endl;
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