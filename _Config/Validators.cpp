/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validators.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nazouz <nazouz@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/23 18:00:37 by nazouz            #+#    #+#             */
/*   Updated: 2025/03/02 23:40:19 by nazouz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

size_t						tokensCounter(const std::string& str) {
	size_t					start = 0, end;
	size_t					count = 0;
	std::string				charset = " \t";
	
	while ((end = str.find_first_of(charset, start)) != std::string::npos) {
		if (end > start)
			count++;
		start = end + 1;
	}
	
	if (start < str.length())
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

// RFC 1034 Section-3.1
bool					Config::isValidServerName(const std::string& serverName, ServerConfig& currentServer) {
	std::string				node;
	std::stringstream		ss(serverName);
	int						nodesCount = 0;
	
	currentServer.server_names.clear();
	if (serverName.empty() || tokensCounter(serverName) < 1 || serverName.size() > 253)
		return false;
	
	while (std::getline(ss, node, '.')) {
		if (++nodesCount > 127)
			return false;
		if (node.empty() || node.size() > 63)
			return false;
		if (!isalnum(node[0]) || !isalnum(node[node.size() - 1]))
			return false;
		for (size_t i = 0; i < node.size(); i++) {
			if (!isalnum(node[i]) && node[i] != '-')
				return false;
		}
	}
	if (nodesCount >= 2)
		currentServer.server_names.push_back(serverName);
	return nodesCount >= 2;
}

bool					Config::isValidErrorPage(const std::string& errorPage, Directives& toFill) {
	if (tokensCounter(errorPage) < 2)
		return false;
	
	std::vector<std::string>	values = split(errorPage, " \t");
	
	for (size_t i = 0; i < values.size() - 1; i++) {
		if (!stringIsDigit(values[i]))
			return false;
		
		size_t	errorCode;
		char	*stringstop;

		errorCode = std::strtoul(values[i].c_str(), &stringstop, 10);
		if (ERANGE == errno || EINVAL == errno)
			return false;
		toFill.error_pages.insert(std::make_pair(errorCode, values.back()));
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
	
	size_t				value = 0;
	char				*stringstop;
	if (isUnit) {
		if (!(unit == 'G' || unit == 'M' || unit == 'K') || !stringIsDigit(client_max_body_size.substr(0, client_max_body_size.size() - 1)))
			return false;
		
		value = std::strtoul(client_max_body_size.substr(0, client_max_body_size.size() - 1).c_str(), &stringstop, 10);
		if (ERANGE == errno || EINVAL == errno)
			return false;

		if (unit == 'K') {
			if (value * KB < value)
				return false;
			value = value * KB;
		}
		else if (unit == 'M') {
			if (value * MB < value)
			return false;
			value = value * MB;
		}
		else if (unit == 'G') {
			if (value * GB < value)
			return false;
			value = value * GB;
		}
	} else {
		if (!stringIsDigit(client_max_body_size))
			return false;
		
		value = std::strtoul(client_max_body_size.c_str(), &stringstop, 10);
		if (ERANGE == errno || EINVAL == errno)
			return false;
	}
	toFill.client_max_body_size = value;
	return value != 0;
}

bool					Config::isValidRoot(const std::string& root, Directives& toFill) {
	if (tokensCounter(root) != 1)
		return false;

	if (root[0] != '/')
		return (ErrorLogger("[DIRECTIVE] directive <root> is invalid : " + root + " should be absolute path!"), false);
	
	struct stat pathStats;
	if (stat(root.c_str(), &pathStats) != 0)
		return (ErrorLogger("[DIRECTIVE] directive <root> is invalid : " + root + " doesn't exist!"), false);
	if (!S_ISDIR(pathStats.st_mode))
		return (ErrorLogger("[DIRECTIVE] directive <root> is invalid : " + root + " should be a directory!"), false);

	toFill.root = root;
	return true;
}

bool					Config::isValidUploadStore(const std::string& upload_store, Directives& toFill) {
	if (tokensCounter(upload_store) != 1)
		return false;
	
	if (upload_store[0] != '/')
		return (ErrorLogger("[DIRECTIVE] directive <upload_store> is invalid : " + upload_store + " should be absolute path!"), false);
	
	struct stat pathStats;
	if (stat(upload_store.c_str(), &pathStats) != 0)
		return (ErrorLogger("[DIRECTIVE] directive <upload_store> is invalid : " + upload_store + " doesn't exist!"), false);
	if (!S_ISDIR(pathStats.st_mode))
		return (ErrorLogger("[DIRECTIVE] directive <upload_store> is invalid : " + upload_store + " should be a directory!"), false);
	
	if (upload_store[upload_store.size() - 1] != '/')
		const_cast<std::string&>(upload_store) += "/";
	toFill.upload_store = upload_store;
	return true;
}

bool					Config::isValidLocation(const std::string& location, std::map<std::string, Directives>& Locations) {
	if (tokensCounter(location) != 1)
		return false;
	
	if (location[0] != '/')
		return false;
	
	if (Locations.find(location) != Locations.end())
		return (ErrorLogger("[LOCATION] duplicate location block : " + location), false);
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

	std::vector<std::string>	values = split(methods, " \t");

	for (size_t i = 0; i < values.size(); i++)
		if (values[i] != "GET" || values[i] != "POST" || values[i] != "DELETE")
			return false;
	toFill.methods = values;
	return true;
}

bool					Config::isValidIndex(const std::string& index, Directives& toFill) {
	if (tokensCounter(index) < 1)
		return false;
	
	toFill.index.clear();

	std::vector<std::string>	values = split(index, " \t");
	
	toFill.index = values;
	return true;
}

bool					Config::isValidRedirect(const std::string& redirect, Directives& toFill) {
	if (tokensCounter(redirect) != 2)
		return false;
	
	std::vector<std::string>	values = split(redirect, " \t");
	
	size_t	value = 0;
	char	*stringstop;

	value = std::strtoul(values[0].c_str(), &stringstop, 10);
	if (ERANGE == errno || EINVAL == errno)
		return false;
	if (!stringIsDigit(values[0]))
		return false;
	
	toFill.redirect.first = value;
	toFill.redirect.second = values[1];
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

bool					Config::isValidCgiExt(const std::string& cgi_ext, Directives& toFill) {
	if (!(tokensCounter(cgi_ext) >= 1 && tokensCounter(cgi_ext) <= 3))
		return false;
	
	toFill.cgi_ext.clear();
	
	std::vector<std::string>	values = split(cgi_ext, " \t");

	for (size_t i = 0; i < values.size(); i++) {
		size_t	colonPos = values[i].find(':');
		if (colonPos == std::string::npos)
			return false;
		std::string key = values[i].substr(0, colonPos);
		std::string value = values[i].substr(colonPos + 1);
		if (key.empty() || value.empty() || key[0] != '.' || value[0] != '/')
			return false;
		if (key != ".py" && key != ".php" && key != ".sh")
			return false;
		if (toFill.cgi_ext.find(key) != toFill.cgi_ext.end())
			return false;
		toFill.cgi_ext[key] = value;
	}
	return true;
}
