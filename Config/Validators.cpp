/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validators.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nazouz <nazouz@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/23 18:00:37 by nazouz            #+#    #+#             */
/*   Updated: 2024/11/25 18:53:31 by nazouz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

int						tokensCounter(const std::string& string) {
	size_t					count = 0;
	std::string				token;
	std::stringstream		ss(string);
	while (ss >> token)
		count++;
	return count;
}

bool			isValidPort(const std::string& port) {
	if (port.empty() || tokensCounter(port) != 1 || port.size() > 5 || !stringIsDigit(port))
		return false;
	
	int		portDecimal = std::atoi(port.c_str());
	if (portDecimal > 0 && portDecimal < 65535)
		return true;
	return false;
}

bool					isValidHost(const std::string& host) {
	std::string					byte;
	std::stringstream			ss(host);
	int							bytesCount = 0;

	if (host.empty() || tokensCounter(host) != 1)
		return false;
	while (std::getline(ss, byte, '.')) {
		if (++bytesCount > 4)
			return false;
		if (byte.empty() || byte.size() > 3)
			return false;
		for (size_t i = 0; i < byte.size(); i++) {
			if (!isdigit(byte[i]))
				return false;
		}
		if (byte.size() > 1 && byte[0] == '0')
			return false;
		int byteDecimal = std::atoi(byte.c_str());
		if (byteDecimal < 0 || byteDecimal > 255)
			return false;
	}
	return bytesCount == 4;
}

bool					isValidServerName(const std::string& serverName) {
	std::string				sub;
	std::stringstream		ss(serverName);
	int						subCount = 0;
	
	if (serverName.empty() || tokensCounter(serverName) < 1 || serverName.size() > 253)
		return false;
	
	while (std::getline(ss, sub, '.')) {
		if (++subCount > 127)
			return false;
		if (sub.empty() || sub.size() > 63)
			return false;
		if (!isalnum(sub[0]) || !isalnum(sub[sub.size() - 1]))
			return false;
		for (size_t i = 0; i < sub.size(); i++) {
			if (!isalnum(sub[i]) && sub[i] != '-')
				return false;
		}
	}
	return subCount >= 2;
}

bool					isValidErrorPage(const std::string& errorPage) {
	if (tokensCounter(errorPage) < 2)
		return false;
	return true;
}

bool					isValidClientMaxBodySize(std::string& client_max_body_size) {
	if (tokensCounter(client_max_body_size) != 1)
		return false;
	
	bool				isUnit = false;
	char				unit = 'c';
	if (isalpha(client_max_body_size[client_max_body_size.size() - 1]))
		isUnit = true, unit = std::toupper(client_max_body_size[client_max_body_size.size() - 1]);
	
	size_t				value;
	if (isUnit) {
		if (unit == 'G' || unit == 'M' || unit == 'K' || !stringIsDigit(client_max_body_size.substr(0, client_max_body_size.size() - 1)))
			return false;
		value = std::atoi(client_max_body_size.substr(0, client_max_body_size.size() - 1).c_str());
		if (unit == 'K')
			value = value * 1024;
		else if (unit == 'M')
			value = value * 1024 * 1024;
		else if (unit == 'G')
			value = value * 1024 * 1024 * 1024;

		std::stringstream	ss;
		ss << value;
		client_max_body_size = ss.str();
	} else if (!isUnit) {
		if (!stringIsDigit(client_max_body_size))
			return false;
	}
	return true;
}

bool					isValidPath(const std::string& path) {
	if (tokensCounter(path) != 1)
		return false;
	return true;
}

bool					isValidMethods(const std::string& methods) {
	if (tokensCounter(methods) < 1 || tokensCounter(methods) > 3)
		return false;

	std::string				token;
	std::stringstream		ss(methods);
	while (ss >> token)
		if (token != "GET" && token != "POST" && token != "DELETE")
			return false;
	return true;
}

bool					isValidIndex(const std::string& index) {
	if (tokensCounter(index) < 1)
		return false;
	return true;
}

bool					isValidRedirect(const std::string& redirect) {
	if (tokensCounter(redirect) != 2)
		return false;
	
	std::string			token;
	std::stringstream	ss(redirect);
	
	ss >> token;
	if (!stringIsDigit(token) || std::atoi(token.c_str()) < 300 || std::atoi(token.c_str()) > 308)
		return false;
	return true;
}

bool					isValidAutoIndex(const std::string& autoindex) {
	if (tokensCounter(autoindex) != 1)
		return false;
	
	std::string				token;
	std::stringstream		ss(autoindex);
	while (ss >> token)
		if (token != "on" && token != "off")
			return false;
	return true;
}

bool					isValidCgiPass(const std::string& cgi_pass) {
	if (tokensCounter(cgi_pass) != 1)
		return false;
	return true;
}
