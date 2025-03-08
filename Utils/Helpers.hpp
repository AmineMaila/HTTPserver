#ifndef HELPERS_HPP
# define HELPLERS_HPP

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <map>
#include <list>

class Disconnect : public std::exception
{
public:
	virtual ~Disconnect() throw() {}
	Disconnect() {}
	Disconnect(std::string msg) : msg(msg) {}

	virtual const char *what() const throw() { return (msg.c_str()); }

private:
	std::string	msg;
};

class CGIRedirect : public std::exception
{
public:
	virtual ~CGIRedirect() throw() {}
	CGIRedirect(std::string location) : location(location) {}

	std::string	location;
};

class Code : public std::exception
{
public:
	virtual ~Code() throw() {}
	Code(int status) : status(status) {}
	Code(int status, std::string location) : status(status), location(location) {}

	const int 			status;
	const std::string	location;
};

std::vector<std::string>	split(const std::string& tosplit, const std::string& charset);
std::string		stringtrim(const std::string& str, const char *set);
bool			isHexa(const std::string& num);
std::string		stringtolower(std::string str);
bool			stringisdigit(const std::string& str);
ssize_t			htoi(const std::string& num);
std::string		getDate( void );
std::string		getDate( void );
std::string		getContentType(const std::string& target, std::map<std::string, std::string>& mimeTypes);
ssize_t			fileLength(std::string& path);
std::string		_toString(int num);
std::string		_toString(long num);
std::string		_toString(unsigned long num);
std::string		generateRandomString( void );
bool			allDigit(std::string str);
std::string		capitalize(const std::string& word);
std::string		toHex(size_t num);
std::string		buildChunk(const char *data, size_t size);
std::string		normalizeURI(const std::string& path);


#endif