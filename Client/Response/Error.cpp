#include "Error.hpp"
#include "Response.hpp"

FatalError::FatalError(const char *msg) : msg(msg) {}

const char *FatalError::what() const throw()
{
	return (msg);
}

CGIRedirectException::~CGIRedirectException() throw() {}

CGIRedirectException::CGIRedirectException(const std::string& location)
{
	this->location = location;
}

void	Response::generateErrorPage(int& status)
{
	headers.clear();
	headers.append("\r\nServer: webserv/1.0");
	headers.append("\r\nDate: " + getDate());
	if (reqCtx->keepAlive)
		headers.append("\r\nConnection: keep-alive");
	else
		headers.append("\r\nConnection: close");

	try
	{
		// if (!reqCtx->_Config)
		// 	throw(status);
	
		std::map<int, std::string>::iterator error_page = reqCtx->_Config->error_pages.find(status);
		if (error_page == reqCtx->_Config->error_pages.end())
			throw(status);

		fillRequestData(error_page->second, *reqCtx);
		bodyFile.open(reqCtx->fullPath.c_str());
		if (!bodyFile.is_open())
			throw(500);

		headers.append("\r\nContent-Type: " + getContentType(reqCtx->fullPath, mimeTypes));
		headers.append("\r\nContent-Length: " + _toString(fileLength(reqCtx->fullPath)));
		headers.append("\r\n\r\n");
		nextState = READ; // remove it is obselete
	}
	catch (int& newStatus)
	{
		status = newStatus; // maybe remove
		buffer = "<!DOCTYPE html>\n"
				"<html>\n"
				"<head><title> " + _toString(newStatus) + " " + statusCodes[newStatus] + " </title></head>\n"
				"<body>\n"
				"<center><h1> " + _toString(newStatus) + " " + statusCodes[newStatus] + " </h1></center>\n"
				"<hr><center>webserv/1.0</center>\n"
				"</body>\n"
				"</html>";
		headers.append("\r\nContent-Type: text/html");
		headers.append("\r\nContent-Length: " + _toString(buffer.size()));
		headers.append("\r\n\r\n");
		headers.append(buffer);
		nextState = DONE;
	}

	headers.insert(0, "HTTP/1.1 " + _toString(status) + " " + statusCodes[status]) ; // status line
	// std::cout << headers << std::endl;
}