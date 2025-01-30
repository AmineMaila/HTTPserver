#include "Response.hpp"

void	Response::generateErrorPage( void )
{
	buffer.clear();
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
		buffer.append("\r\nContent-Type: text/html");
		buffer.append("\r\nContent-Length: " + _toString(error.size()));
		buffer.append(error);
	nextState = ERROR;
}