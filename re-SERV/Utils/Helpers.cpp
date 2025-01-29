/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Helpers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmaila <mmaila@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/13 16:28:26 by nazouz            #+#    #+#             */
/*   Updated: 2025/01/29 22:00:43 by mmaila           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Helpers.hpp"

// bool			Request::printParsedRequest() {
// 	std::cout << "*********************************************" << std::endl;
// 	// printf("/---------------- REQUEST ----------------/\n\n");
// 	// for (size_t i = 0; i < rawRequest.size(); i++) {
// 	// 	printf("[%s]\n", rawRequest[i].c_str());
// 	// }

// 	// startline
// 	printf("/---------------- REQUESTLINE ----------------/\n");
// 	// printf("RAW STARTLINE: [%s]\n", requestLine.rawRequestLine.c_str());
// 	printf("METHOD: [%s]\n", requestLine.method.c_str());
// 	printf("URI: [%s]\n", requestLine.uri.c_str());
// 	printf("HTTP VERSION: [%s]\n\n", requestLine.httpversion.c_str());

// 	// header
// 	printf("/----------------- HEADERS -----------------/\n");
// 	// printf("RAW HEADERS: [%s]\n", header.rawHeader.c_str());
// 	std::map<std::string, std::string>::iterator it = header.headersMap.begin();
// 	std::map<std::string, std::string>::iterator ite = header.headersMap.end();
// 	while (it != ite) {
// 		printf("[%s][%s]\n", it->first.c_str(), it->second.c_str());
// 		it++;
// 	}
	
// 	// body
// 	// printf("\n/------------------- BODY -------------------/\n");
// 	// printf("RAW BODY: [%s]\n", body.rawBody.c_str());
// 	std::cout << "*********************************************" << std::endl;
// 	return true;
// }

// std::string		stringtrim(const std::string& str, const std::string& set) {
// 	size_t		first = 0;
// 	size_t		last = str.size();

// 	while (!strchr(set.c_str(), str[first]))
// 		first++;
// 	while (!strrchr(set.c_str(), str[last]))
// 		last--;
// 	return str.substr(first, last - first);
// }

std::string			stringtrim(const std::string& str, const std::string& set) {
    if (str.empty() || set.empty()) {
        return str; // Nothing to trim
    }

    size_t first = str.find_first_not_of(set);
    if (first == std::string::npos) {
        return ""; // Entire string consists of characters from `set`
    }

    size_t last = str.find_last_not_of(set);
    return str.substr(first, last - first + 1);
}

std::string		stringtolower(std::string& str) {
	for (size_t i = 0; i < str.size(); i++) {
		if (str[i] >= 'A' && str[i] <= 'Z')
			str[i] += 32;
	}
	return str;
}

bool			stringIsDigit(const std::string& str) {
	for (size_t i = 0; i < str.size(); i++) {
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}

bool			isHexa(const std::string& num) {
	std::string		hex = "0123456789ABCDEFabcdef";

	for (size_t i = 0; i < num.size(); i++) {
		if (hex.find(num[i]) == std::string::npos)
			return false;
	}
	return true;
}

int				hexToInt(const std::string& num) {
	int						result;
	std::stringstream		ss;

	ss << std::hex << num;
	ss >> result;
	return result;
}

unsigned int	parseIPv4(const std::string& ipAddress) {
	std::stringstream	ss(ipAddress);
	unsigned int		octets;
	unsigned char		*ptr = (unsigned char *)&octets;

	std::string			token;
	while (std::getline(ss, token, '.')) {
		*ptr = std::atoi(token.c_str());
		ptr++;
	}
	return octets;
}

long	fileLength(std::string& path)
{
	struct stat fileStat;
	if (stat(path.c_str(), &fileStat) == 0)
		return (fileStat.st_size);
	return (-1);
}

std::string	getContentType(const std::string& target, std::map<std::string, std::string>& mimeTypes)
{
	std::map<std::string, std::string>::iterator it;
	std::string ext;
	std::string file = target.substr(target.find_last_of('/'));
	ext = file.substr(file.find('.'));
	it = mimeTypes.find(ext);
	if (it != mimeTypes.end())
		return (it->second);
	else
		return ("text/plain");
}

std::string	getDate( void )
{
    char buffer[100];

	std::time_t now = std::time(0);
    std::tm* gmt = std::gmtime(&now);

    std::strftime(buffer, 100, "%a, %d %b %Y %H:%M:%S GMT", gmt);
	return (buffer);
}

std::string		_toString(long num)
{
	std::ostringstream ret;
	ret << num;
	return (ret.str());
}

std::string		_toString(unsigned long num)
{
	std::ostringstream ret;
	ret << num;
	return (ret.str());
}

std::string		_toString(int num)
{
	std::ostringstream ret;
	ret << num;
	return (ret.str());
}

bool	rootJail(const std::string& uri)
{
	int					traverse = 0;
	std::stringstream	target(uri);
	std::string			token;
	while (getline(target, token, '/'))
	{
		if (token == "..")
			traverse--;
		else
			traverse++;
	}
	if (traverse < 0)
		return (false);
	return (true);
}

bool	allDigit(std::string str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		if (!isdigit(str[i]))
			return (false);
	}
	return (true);
}

std::string		generateRandomString( void )
{
	std::string alphaNum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	srand(static_cast<size_t>(time(0)));

	std::string random;
	for (int i = 0; i < 10; i++)
	{
		random += alphaNum[rand() % 61];
	}
	return (random);
}

std::string		toHex(size_t num)
{
	std::stringstream ss;

	ss << std::uppercase << std::hex << num;
	return (ss.str());
}

// #include <arpa/inet.h>
// int main() {
// 	unsigned int my = parseIPv4("192.168.1.1");
// 	printf("mine = %d\n", my);
// 	printf("mine = %d\n", htonl(my));
// 	unsigned int ip = 0;
// 	inet_pton(AF_INET, "192.168.1.1", &ip);
// 	printf("ip = %d\n", ip);
// }