#include "Error.hpp"

FatalError::FatalError(std::string msg) : msg(msg) {}

ErrorPage::ErrorPage(int code) : status(code)
{
    description.insert(std::make_pair(400, "Bad Request"));
    description.insert(std::make_pair(403, "Forbidden"));
    description.insert(std::make_pair(404, "Not Found"));
    description.insert(std::make_pair(405, "Method Not Allowed"));
    description.insert(std::make_pair(413, "Payload Too Large"));
    description.insert(std::make_pair(415, "Unsupported Media Type"));
    description.insert(std::make_pair(416, "Range Not Satisfiable"));
    description.insert(std::make_pair(431, "Request Header Fields Too Large"));
    description.insert(std::make_pair(500, "Internal Server Error"));
    description.insert(std::make_pair(501, "Not Implemented"));
    description.insert(std::make_pair(504, "Gateway Timeout"));
    description.insert(std::make_pair(505, "HTTP Version Not Supported"));
}

std::string	ErrorPage::getBuffer( void ) const
{
	return (buffer);
}

void	ErrorPage::generateErrorPage( void )
{
	buffer = ("HTTP/1.1 " + _toString(status) + " " + description[status]); // status line
	buffer.append("\r\nServer: webserv/1.0");
	buffer.append("\r\nDate: " + getDate());

	 // if (error_page directive exists)
	 	// open the error page in bodyFD
		// content-type and content length hna
	 //else
		std::string error;
		error = "<!DOCTYPE html>\n"
				"<html>\n"
				"<head>\n"
				"    <title> " + _toString(status) + " " + description[status] + " </title>\n"
				"</head>\n"
				"<body>\n"
				"    <center>\n"
				"        <h1> " + _toString(status) + " " + description[status] + " </h1>\n"
				"    </center>\n"
				"    <hr>\n"
				"    <center>webserv/1.0</center>\n"
				"</body>\n"
				"</html>";
		buffer.append("\r\nContent-Type: text/html");
		buffer.append("\r\nContent-Length: " + _toString(error.size()));
		buffer.append(error);
}

const char *FatalError::what() const throw()
{
	return (msg.c_str());
}