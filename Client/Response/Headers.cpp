#include "Response.hpp"

void	Response::handlePOST()
{
	buffer = "<!DOCTYPE html>\n"
			"<html>\n"
			"<head>\n"
			"<title>Upload Successful</title>\n"
			"</head>\n"
			"<body>\n"
			"<center><h1>Upload Successful</h1></center>\n"
			"<hr width=\"100%\" size=\"1\" color=\"black\">\n"
			"<center><p>data has been uploaded successfully.</p></center>\n"
			"</body>\n"
			"</html>\n";
	headers.append("\r\nContent-Type: text/html");
	headers.append("\r\nContent-Length: " + _toString(buffer.size()));
	nextState = DONE;
}

void	Response::handleDELETE( void )
{
	if (reqCtx->isDir)
	{
		if (rmdir(reqCtx->fullPath.c_str()) == -1)
			throw(Code(500));
	}
	else
	{
		if (remove(reqCtx->fullPath.c_str()) == -1)
			throw(Code(500));
	}
	reqCtx->StatusCode = 204;
	nextState = DONE;
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
		bodyFile.open(reqCtx->fullPath.c_str());
		if (!bodyFile.is_open())
			throw(Code(500));
		contentType = getContentType(reqCtx->fullPath, mimeTypes);
		contentLength = fileLength(reqCtx->fullPath);
	
		if (reqCtx->Headers.find("range") != reqCtx->Headers.end())
			handleRange();
		headers.append("\r\nContent-Length: " + _toString(contentLength));
	}
	headers.append("\r\nContent-Type: " + contentType);
	headers.append("\r\nAccept-Ranges: bytes");
}

void	Response::generateHeaders( void )
{
	headers.append("\r\nServer: webserv/1.0");
	headers.append("\r\nDate: " + getDate());

	if (reqCtx->Method == "GET")
		handleGET();
	else if (reqCtx->Method == "DELETE")
		handleDELETE();
	else if (reqCtx->Method == "POST")
		handlePOST();

	if (reqCtx->keepAlive)
		headers.append("\r\nConnection: keep-alive");
	else
		headers.append("\r\nConnection: close");

	headers.insert(0, "HTTP/1.1 " + _toString(reqCtx->StatusCode) + " " + getCodeDescription(reqCtx->StatusCode)); // status line
	headers.append("\r\n\r\n");
	if (reqCtx->Method == "POST")
		headers.append(buffer);
	if ((this->*sender)() == true)
		state = nextState;
	else
		state = WRITE;
}