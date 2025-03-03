/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Helpers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmaila <mmaila@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/13 16:28:26 by nazouz            #+#    #+#             */
/*   Updated: 2025/03/03 00:42:38 by mmaila           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Helpers.hpp"


std::vector<std::string>	split(const std::string& tosplit, const std::string& charset) {
	std::vector<std::string> result;
	size_t start = 0, end;
	
	while ((end = tosplit.find_first_of(charset, start)) != std::string::npos) {
		if (end > start) {
			result.push_back(tosplit.substr(start, end - start));
		}
		start = end + 1;
	}
	
	if (start < tosplit.length())
		result.push_back(tosplit.substr(start));
	return result;
}

std::string			stringtrim(const std::string& str, const char *set) {
	if (str.empty() || !set) {
		return (str);
	}

	size_t first = str.find_first_not_of(set);
	if (first == std::string::npos) {
		return "";
	}

	size_t last = str.find_last_not_of(set);
	return str.substr(first, last - first + 1);
}

std::string		stringtolower(std::string str) {
	for (size_t i = 0; i < str.size(); i++) {
		str[i] = std::tolower(str[i]);
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
	static const char	hex[] = "0123456789ABCDEFabcdef";
	
	if (num.find_first_not_of(hex) != std::string::npos)
		return (false);
	return (true);
}

ssize_t htoi(const std::string& num)
{
	char *stop;
	unsigned long value = std::strtoul(num.c_str(), &stop, 16);
	if (ERANGE == errno || EINVAL == errno)
		return -1;

	return (static_cast<ssize_t>(value));
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
	
	size_t pos;
	pos = file.find('.');
	if (pos != std::string::npos)
	{
		ext = file.substr(pos);
		it = mimeTypes.find(ext);
		if (it != mimeTypes.end())
			return (it->second);
	}
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
	static const char alphaNum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	std::string random;
	for (int i = 0; i < 10; i++)
	{
		random += alphaNum[rand() % 61];
	}
	return (random);
}

std::string capitalize(const std::string& input)
{
	std::string output = input;

	static const char alpha[] = "abcdefghijklmnopqrstuvwxyz";
	size_t pos = 0;

	while ((pos = output.find_first_of(alpha, pos)) != std::string::npos)
	{
		output[pos++] -= 32;
		pos = output.find_first_not_of(alpha, pos);
	}
	
	return (output);
}

inline std::string toHex(size_t size)
{
	if (size == 0)
		return "0";
	
	std::string	result;
	static char base16[] = "0123456789ABCDEF";
	

	while (size > 0)
	{
		result = base16[size % 16] + result;
		size /= 16;
	}
	
	return (result);
}

std::string buildChunk(const char* data, size_t size)
{
	if (!data || size == 0)
		return "0\r\n\r\n";
	
	std::string hex = toHex(size);
	std::string result;

	result.reserve(hex.length() + 2 + size + 2);
	
	result.append(hex);
	result.append("\r\n", 2);
	result.append(data, size);
	result.append("\r\n", 2);
	
	return (result);
}

// std::string	buildChunk(const char *data, size_t size) // error
// {
// 	return (toHex(size) + "\r\n" + std::string(data, size) + "\r\n");
// }

/////////////////////////////////////////////////////
void printMap(std::map<std::string, std::string>& map)
{
	std::cout << "************************************" << std::endl;
	std::map<std::string, std::string>::iterator it;
	for (it = map.begin(); it != map.end(); it++)
		std::cout << it->first << "::" << it->second << std::endl;
	std::cout << "************************************" << std::endl;
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