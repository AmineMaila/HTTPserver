#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <sys/socket.h>
# include <sys/stat.h>
# include <unistd.h>
# include <string.h>
# include <cstdio>
# include <algorithm>
# include <dirent.h>
# include <cstdlib>
# include <iostream>
# include <fstream>
# include <map>
# include <vector>
# include "../../Utils/Helpers.hpp"
# include "../../Config/Config.hpp"

# define SEND_BUFFER_SIZE 16384
# define MAX_CONCURRENT_PROCESSES 50

enum	State
{
	READBODY,
	READCHUNK,
	LISTDIR,
	NEXTRANGE,
	READRANGE,
	SENDDATA,
	FINISHED
};

struct Range
{
	std::pair<int, int> range;
	std::string			header;
	size_t				rangeLength;
	bool				headerSent = false;
};

struct	CgiInput
{
	// not used only for refrence in developement
	pid_t		pid;
	int			fd; // file descriptor where cgi writes its output into
	bool		isCgi = false; // is requested resource CGI
	std::string	interpreter; // path to the cgi interpreter ex. /usr/bin/python3 for .py
	std::string	ext; // cgi script extention
	std::string	scriptName;
	std::string	pathInfo;
	std::string	queryString;
};

struct	ResponseInput
{
	int									status;
	std::string							method;
	std::string							uri;
	std::string							path;
	bool								isDir = false;
	bool								isCgi = false;
	std::map<std::string, std::string>	requestHeaders;
	Directives							config;
	// struct CgiInput						cgi;
};

class Response
{
public:
	~Response();
	Response();
	Response(const Response& rhs);
	Response&	operator=(const Response& rhs);

	// external functions
	void		setInput(struct ResponseInput& input);
	void		setBuffer(const std::string& data);
	int			getStatusCode() const;

	int			sendResponse( int& socket );
	void		generateHeaders( void );


	// sending body
	void		openBodyFile(const std::string& path);
	void		readBody();
	std::string	buildChunk(const char *data, size_t size);
	void		readChunk();
	void		readRange();
	void		buildRange( void );
	void		getNextRange();
	bool		sendData(int& socket);

	// autoindex
	void		autoIndex();
	void		directoryListing();

private:
	// range parsing
	int				rangeContentLength( void );
	unsigned long	parseRangeValue(std::string& value);
	bool			parseRangeHeader( void );
	
	// Methods
	void		handleGET( void );
	void		handlePOST( void );
	void		handleDELETE( void );


	// response needed data
	struct ResponseInput	input;

	// response constants
	std::map<std::string, std::string>	mimeTypes;
	std::map<int, std::string>			statusCodes;


	// response frequent uses
	std::string		contentType;
	unsigned long	contentLength;


	// response creating process
	std::string		headers;
	std::ifstream	bodyFile;
	DIR				*dirList; // might produce leaks

	// range
	std::vector<Range>	ranges;
	size_t				currRange;
	std::string			boundary;

	// response state
	enum State	state;
	enum State	nextState;

	// response buffer
	std::string buffer;
};

#endif