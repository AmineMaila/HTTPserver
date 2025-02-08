#include "Response.hpp"
#include "Error.hpp"

int	Response::rangeContentLength( void )
{
	int length = 0;
	std::vector<Range>::iterator it = reqCtx->rangeData.ranges.begin();

	for (; it != reqCtx->rangeData.ranges.end(); it++)
	{
		length += it->header.length();
		length += it->rangeLength;
	}
	length += 8 + reqCtx->rangeData.boundary.length(); // 8 is  the length of the constant "\r\n--" "--\r\n" // FIX
	return (length);
}

void	Response::handleRange()
{
	if (reqCtx->rangeData.ranges.size() == 1)
	{
		contentLength = reqCtx->rangeData.current->rangeLength;
		headers.append("\r\nContent-Range: bytes "
			+ _toString(reqCtx->rangeData.current->range.first)
			+ "-"
			+ _toString(reqCtx->rangeData.current->range.second)
			+ "/"
			+ _toString(contentLength));
	}
	else
	{
		contentType = "multipart/byteranges; boundary=" + reqCtx->rangeData.boundary;
		contentLength = rangeContentLength();
	}
	reader = &Response::readRange;
}

void	Response::handleGET( void )
{
	if (reqCtx->_Config->autoindex && reqCtx->isDir)
	{
		initDirList();
		headers.append("\r\nTransfer-Encoding: chunked");
		contentType = "text/html";
		reader = &Response::directoryListing;
	}
	else
	{
		bodyFile.open(reqCtx->fullPath.c_str()); // no protection
		if (!bodyFile.is_open())
			throw(500);
		contentType = getContentType(reqCtx->fullPath, mimeTypes);
		contentLength = fileLength(reqCtx->fullPath);
	
		if (reqCtx->isRange)
			handleRange();
		headers.append("\r\nContent-Length: " + _toString(contentLength));
	}
	headers.append("\r\nContent-Type: " + contentType);
	headers.append("\r\nAccept-Ranges: bytes");
}

void	Response::generateHeaders( void )
{
	headers = "HTTP/1.1 " + _toString(reqCtx->StatusCode) + " " + statusCodes[reqCtx->StatusCode]; // status line
	headers.append("\r\nServer: webserv/1.0");
	headers.append("\r\nDate: " + getDate());

	if (reqCtx->Method == "GET") // check allowed methods
		handleGET();
	// else if (reqCtx->Method == "POST")
	// 	handlePOST();
	else if (reqCtx->Method == "DELETE")
		nextState = DONE;

	if (reqCtx->keepAlive)
		headers.append("\r\nConnection: keep-alive");
	else
		headers.append("\r\nConnection: close");

	headers.append("\r\n\r\n");
	sender = &Response::sendHeaders;
}