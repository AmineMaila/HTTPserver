#ifndef ARES_HPP
# define ARES_HPP

# include <map>
# include <dirent.h>
# include <sys/socket.h>
# include "../Request/Request.hpp"

# define SEND_BUFFER_SIZE 16384

enum State
{
	HEADERS,
	READ,
	WRITE,
	DONE
};

class AResponse
{
public:
	virtual ~AResponse() {}
	AResponse(int &clientSocket, RequestData *data) : socket(clientSocket), state(HEADERS), nextState(READ), reqCtx(data)
	{
		statusCodes.insert(std::make_pair(200, "OK"));
		statusCodes.insert(std::make_pair(201, "Created"));
		statusCodes.insert(std::make_pair(204, "No Content"));
		statusCodes.insert(std::make_pair(206, "Partial Content"));

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

		sender = &AResponse::sendHeaders;
	}

	virtual void	generateHeaders( void ) = 0;
	virtual int		respond() = 0;

	std::string	getCodeDescription(int code)
	{
		std::map<int, std::string>::iterator	it = statusCodes.find(code);
		if (it != statusCodes.end())
			return (it->second);
		return ("");
	}

	bool	sendHeaders()
	{
		ssize_t bytesSent = send(socket, headers.c_str(), headers.size(), 0);
		if (bytesSent == -1)
			throw(Disconnect("\tClient " + _toString(socket) + " : send: " + strerror(errno)));
		else if (bytesSent == 0 && !headers.empty())
			throw(Disconnect("\tClient " + _toString(socket) + " : send: unable to send"));
		headers.erase(0, bytesSent);
		if (headers.empty())
			sender = &AResponse::sendBody;
		return (headers.empty());
	}

	bool	sendBody()
	{
		ssize_t bytesSent = send(socket, buffer.c_str(), buffer.size(), 0);
		if (bytesSent == -1)
			throw(Disconnect("\tClient " + _toString(socket) + " : send: " + strerror(errno)));
		else if (bytesSent == 0 && !buffer.empty())
			throw(Disconnect("\tClient " + _toString(socket) + " : send: unable to send"));
		buffer.erase(0, bytesSent);
		return (buffer.empty());
	}

protected:
	int				socket;
	enum State		state;
	enum State		nextState;
	std::string		headers;
	std::string		buffer;
	bool			(AResponse::*sender)();
	RequestData		*reqCtx;
	std::map<int, std::string>			statusCodes;
	std::map<std::string, std::string>	mimeTypes;
};

#endif