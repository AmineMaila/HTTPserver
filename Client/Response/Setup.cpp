#include "Response.hpp"
#include "Error.hpp"

void	Response::openBodyFile(const std::string& path)
{
	bodyFile.open(path);
	if (!bodyFile.is_open())
		throw(FatalError("could not open requested resource"));
}

void	Response::generateHeaders( void )
{
	headers.append("\r\nServer: webserv/1.0");
	headers.append("\r\nDate: " + getDate());

	if (input.status >= 400)
		throw(ErrorPage(input.status)); // temporary until request throws exception
	else if (input.method == "GET") // check allowed methods
		handleGET();
	else if (input.method == "POST")
		handlePOST();
	else if (input.method == "DELETE")
		handleDELETE();

	if (input.requestHeaders.find("connection") != input.requestHeaders.end())
		headers.append("\r\nConnection: " + input.requestHeaders["connection"]);

	headers.append("\r\n\r\n");

	headers = ("HTTP/1.1 " + _toString(input.status) + " " + statusCodes[input.status]) + headers; // status line
	buffer.insert(0, headers);
	nextState = state;
}