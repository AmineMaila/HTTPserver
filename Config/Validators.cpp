/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validators.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmaila <mmaila@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/23 18:00:37 by nazouz            #+#    #+#             */
/*   Updated: 2025/02/01 14:52:03 by mmaila           ###   ########.fr       */
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

bool			Config::isValidPort(const std::string& port, ServerConfig& currentServer) {
	if (port.empty() || tokensCounter(port) != 1 || port.size() > 5 || !stringIsDigit(port))
		return false;
	
	int		portDecimal = std::atoi(port.c_str());
	if (portDecimal > 0 && portDecimal < 65535) {
		currentServer.port = port;
		return true;
	}
	return false;
}

bool					Config::isValidHost(const std::string& host, ServerConfig& currentServer) {
	std::string					byte;
	std::stringstream			ss(host);
	int							bytesCount = 0;
	currentServer.host = host;
	return true;
	
	if (host == "localhost") {
		currentServer.host = host;
		return true;
	}

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
	
	if (bytesCount == 4)
		currentServer.host = host;
	return bytesCount == 4;
}

bool					Config::isValidServerName(const std::string& serverName, ServerConfig& currentServer) {
	std::string				sub;
	std::stringstream		ss(serverName);
	int						subCount = 0;
	
	currentServer.server_names.clear();
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
	if (subCount >= 2)
		currentServer.server_names.push_back(serverName);
	return subCount >= 2;
}

bool					Config::isValidErrorPage(const std::string& errorPage, Directives& toFill) {
	size_t		tokensCount = tokensCounter(errorPage);
	if (tokensCount < 2)
		return false;
	
	toFill.error_pages.clear();
	std::string					token;
	std::vector<std::string>	tokens;
	std::stringstream			ss(errorPage);
	while (ss >> token)
		tokens.push_back(token);
	
	for (size_t i = 0; i < tokens.size() - 1; i++) {
		if (!stringIsDigit(tokens[i]))
			return false;
		toFill.error_pages.push_back(std::make_pair(std::atoi(tokens[i].c_str()), tokens.back()));
	}
	return true;
}

bool					Config::isValidClientMaxBodySize(const std::string& client_max_body_size, Directives& toFill) {
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

		// std::stringstream	ss;
		// ss << value;
		// client_max_body_size = ss.str();
	} else if (!isUnit) {
		if (!stringIsDigit(client_max_body_size))
			return false;
	}
	toFill.client_max_body_size = value;
	return true;
}

bool					Config::isValidRoot(const std::string& root, Directives& toFill) {
	if (tokensCounter(root) != 1)
		return false;
	std::cout << "isValidRoot = " << &toFill << std::endl;
	toFill.root = root;
	return true;
}

bool					Config::isValidUploadStore(const std::string& upload_store, Directives& toFill) {
	if (tokensCounter(upload_store) != 1)
		return false;
	toFill.upload_store = upload_store;
	return true;
}

bool					Config::isValidLocation(const std::string& location) {
	if (tokensCounter(location) != 1)
		return false;
	
	// currentServer.Locations.push_back(std::make_pair(location, Directives()));
	return true;
}

bool					Config::isValidAlias(const std::string& alias, Directives& toFill) {
	if (tokensCounter(alias) != 1)
		return false;
	toFill.alias = alias;
	return true;
}

bool					Config::isValidMethods(const std::string& methods, Directives& toFill) {
	if (tokensCounter(methods) < 1 || tokensCounter(methods) > 3)
		return false;

	toFill.methods.clear();
	std::string				token;
	std::stringstream		ss(methods);
	while (ss >> token) {
		if (token != "GET" && token != "POST" && token != "DELETE")
			return false;
		toFill.methods.push_back(token);
	}
	return true;
}

bool					Config::isValidIndex(const std::string& index, Directives& toFill) {
	if (tokensCounter(index) < 1)
		return false;
	
	toFill.index.clear();
	// should tokenize first (split by spaces)
	toFill.index.push_back(index);
	return true;
}

bool					Config::isValidRedirect(const std::string& redirect, Directives& toFill) {
	if (tokensCounter(redirect) != 2)
		return false;
	
	std::string			token;
	std::stringstream	ss(redirect);
	
	ss >> token;
	if (!stringIsDigit(token) || std::atoi(token.c_str()) < 300 || std::atoi(token.c_str()) > 308)
		return false;
	toFill.redirect.first = std::atoi(token.c_str());
	ss >> token;
	toFill.redirect.second = token;
	return true;
}

bool					Config::isValidAutoIndex(const std::string& autoindex, Directives& toFill) {
	if (tokensCounter(autoindex) != 1)
		return false;

	if (autoindex != "on" && autoindex != "off")
		return false;
	toFill.autoindex = (autoindex == "off") ? false : true;
	return true;
}

bool					Config::isValidCgiExt(const std::string& cgi_pass, Directives& toFill) {
	if (tokensCounter(cgi_pass) != 1)
		return false;
	toFill.cgi_ext.clear();
	// toFill.cgi_ext[key] = value;
	return true;
}
