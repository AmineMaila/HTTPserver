#include "../Response.hpp"
#include "../Error.hpp"

std::string	Response::buildChunk(const char *data, size_t size) // error
{
	std::string chunk;

	chunk = toHex(size) + "\r\n" + std::string(data, size) + "\r\n";
	if (nextState == FINISHED)
		chunk.append("0\r\n\r\n");
	return (chunk);
}

void	Response::readChunk()
{
	char buf[SEND_BUFFER_SIZE] = {0};
	int bytesRead = bodyFile.read(buf, SEND_BUFFER_SIZE).gcount();
	// std::cout << bytesRead << std::endl;
	if (bytesRead == -1)
	{
		throw (FatalError(strerror(errno)));
	}
	if (bytesRead > 0)
	{
		if (bodyFile.peek() == EOF)
			nextState = FINISHED;
		buffer.append(buildChunk(buf, bytesRead));
		state = SENDDATA;
	}
}

void	Response::readBody()
{
	char buf[SEND_BUFFER_SIZE] = {0};
	int bytesRead = bodyFile.read(buf, SEND_BUFFER_SIZE).gcount();
	// std::cout << bytesRead << std::endl;
	if (bytesRead == -1)
	{
		throw (FatalError(strerror(errno)));
	}
	if (bytesRead > 0)
	{
		buffer.append(std::string(buf, bytesRead));
		state = SENDDATA;
		if (bodyFile.peek() == EOF)
			nextState = FINISHED;
	}
}

void	Response::handleGET( void )
{
	if (input.config.autoindex && input.isDir)
	{
		autoIndex();
		headers.append("\r\nTransfer-Encoding: chunked");
		contentType = "text/html";
		state = LISTDIR;
	}
	else
	{
		contentType = getContentType(input.path, mimeTypes);
		contentLength = fileLength(input.path);
		if (input.requestHeaders.find("Range") != input.requestHeaders.end())
		{
			std::cout << "Hello" << std::endl;
			buildRange();
			
		}
		else if (contentType.find("video") != std::string::npos || contentType.find("audio") != std::string::npos)
		{
			headers.append("\r\nTransfer-Encoding: chunked");
			state = READCHUNK;
		}
		else
			headers.append("\r\nContent-Length: " + _toString(contentLength));
		openBodyFile(input.path);
	}
	headers.append("\r\nContent-Type: " + contentType);
	headers.append("\r\nAccept-Ranges: bytes");
}
