#include "../Response.hpp"

void	Response::buildChunk()
{
	buffer = toHex(buffer.size()) + "\r\n" + buffer + "\r\n";
	if (nextState == FINISHED)
		buffer.append("0\r\n\r\n");
	state = SENDDATA;
}

void	Response::readBody()
{
	char buf[SEND_BUFFER_SIZE] = {0};
	int bytesRead = bodyFile.read(buf, SEND_BUFFER_SIZE).gcount();
	if (bytesRead > 0)
	{
		// std::cout << "BYTESREAD : "<< bytesRead << std::endl;
		// buffer[bytesRead] = '\0'; // be CAREFUL
		buffer.append(std::string(buf, bytesRead));
		// std::cout << "DATA SIZE AFTER READ: " << buffer.size() << std::endl;
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

bool	Response::getResource( void )
{
	// cgi check here


	bodyFile.open(input.path);
	if (!bodyFile.is_open())
	{
		input.status = 500;
		return (false);
	}

	contentType = getContentType(input.path, mimeTypes);
	contentLength = fileLength(input.path);
	return (true);
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
		dirList = opendir(input.path.c_str()); // CHECK LEAK
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
