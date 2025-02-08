#ifndef HELPERS_HPP
# define HELPLERS_HPP

#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <sys/stat.h>
#include <map>


// struct Directives
// {
// 	std::string 								root;
// 	std::string 								alias;
// 	bool 										autoindex;
// 	std::string 								upload_store;
// 	std::string 								cgi_pass;
// 	std::vector<std::string>					index;
// 	std::vector<std::string>					methods;
// 	std::vector<std::string>					cgi_ext;
// 	std::pair<int, std::string> 				redirect;
// 	std::vector<std::pair<int, std::string> >	error_page;
// };

std::string		stringtrim(const std::string& str, const std::string& set);
bool			isHexa(const std::string& num);
std::string		stringtolower(std::string str);
bool			stringIsDigit(const std::string& str);
int				hexToInt(const std::string& num);
unsigned int	parseIPv4(const std::string& ipAddress);
std::string		getDate( void );
std::string		getDate( void );
std::string		getContentType(const std::string& target, std::map<std::string, std::string>& mimeTypes);
long			fileLength(std::string& path);
std::string		_toString(int num);
std::string		_toString(long num);
std::string		_toString(unsigned long num);
bool			rootJail(const std::string& uri);
std::string		generateRandomString( void );
bool			allDigit(std::string str);
void			capitalize(std::string& word);
std::string		toHex(size_t num);

#endif