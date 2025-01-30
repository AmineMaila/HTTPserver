/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Blocks.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nazouz <nazouz@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/25 15:57:29 by nazouz            #+#    #+#             */
/*   Updated: 2025/01/19 17:48:24 by nazouz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

bool				Config::parseAllServerBlocks() {
	for (size_t i = 0; i < serverBlocksIndexes.size(); i++) {
		int						start, end;
		vServerConfigParser		newServerParser;
		
		start = serverBlocksIndexes[i].first;
		end = serverBlocksIndexes[i].second;
		if (!parseSingleServerBlock(start, end, newServerParser)) {
			Logger("'server' block isn't valid (ignored)");
			continue;
		}
		// Logger("'server' block is valid");
		Parser.push_back(newServerParser);
	}
	return Parser.size() >= 1;
}

bool				Config::parseSingleServerBlock(int start, int end, vServerConfigParser& currentServerParser) {
	for (int i = start + 1; i < end; i++) {
		if (configFileVector[i] == "[location]") {
			if (!parseSingleLocationBlock(i, getBlockEndIndex(i, "[location]").second, currentServerParser))
				return false;
			i = getBlockEndIndex(i, "[location]").second;
			continue;
		}

		size_t	equalsPos = configFileVector[i].find('=');
		if (equalsPos == std::string::npos)
			return (Logger("'server' invalid line syntax : '" + configFileVector[i] + "'"), false);
		std::string key = stringtrim(configFileVector[i].substr(0, equalsPos), " \t");
		std::string value = stringtrim(configFileVector[i].substr(equalsPos + 1), " \t");
		if (key.empty() || value.empty())
			return (Logger("'server' invalid line syntax : '" + configFileVector[i] + "'"), false);
		if (!isAllowedDirective(key, "server"))
			return (Logger("'server' unknown '" + key + "' directive : '" + configFileVector[i] + "'"), false);
		if (!addToServerParserServerDirectives(key, value, currentServerParser.serverDirectives))
			return (Logger("'server' duplicate '" + key + "' directive : '" + configFileVector[i] + "'"), false);
	}
	return validateServerBlockDirectives(currentServerParser.serverDirectives);
}

bool				Config::parseSingleLocationBlock(int start, int end, vServerConfigParser& currentServerParser) {
	std::map<std::string, std::string>		newLocationMap;
	
	for (int i = start + 1; i < end; i++) {
		size_t	equalsPos = configFileVector[i].find('=');
		if (equalsPos == std::string::npos)
			return (Logger("'location' invalid line syntax : '" + configFileVector[i] + "'"), false);
		std::string key = stringtrim(configFileVector[i].substr(0, equalsPos), " \t");
		std::string value = stringtrim(configFileVector[i].substr(equalsPos + 1), " \t");
		if (key.empty() || value.empty())
			return (Logger("'location' invalid line syntax : '" + configFileVector[i] + "'"), false);
		if (!isAllowedDirective(key, "location"))
			return (Logger("'location' unknown '" + key + "' directive : '" + configFileVector[i] + "'"), false);
		if (!addToServerParserLocationDirectives(key, value, newLocationMap))
			return (Logger("'location' duplicate '" + key + "' directive : '" + configFileVector[i] + "'"), false);
	}
	currentServerParser.locationDirectives.push_back(newLocationMap);
	return validateLocationBlockDirectives(currentServerParser.locationDirectives.back());
}

bool				Config::validateServerBlockDirectives(std::map<std::string, std::string>& directives) {
	if (directives.find("host") != directives.end() && !isValidHost(directives["host"]))
		return (Logger("'host' directive is invalid in 'server block' :  \'" + directives["host"] + "\'"), false);
	
	if (directives.find("port") != directives.end() && !isValidPort(directives["port"]))
		return (Logger("'port' directive is invalid in 'server block' :  \'" + directives["port"] + "\'"), false);
	
	if (directives.find("server_name") != directives.end() && !isValidServerName(directives["server_name"]))
		return (Logger("'server_name' directive is invalid in 'server block' :  \'" + directives["server_name"] + "\'"), false);
	
	if (directives.find("error_page") != directives.end() && !isValidErrorPage(directives["error_page"]))
		return (Logger("'error_page' directive is invalid in 'server block' :  \'" + directives["error_page"] + "\'"), false);
	
	if (directives.find("client_max_body_size") != directives.end() && !isValidClientMaxBodySize(directives["client_max_body_size"]))
		return (Logger("'client_max_body_size' directive is invalid in 'server block' :  \'" + directives["client_max_body_size"] + "\'"), false);
	
	if (directives.find("root") != directives.end() && !isValidPath(directives["root"]))
		return (Logger("'root' directive is invalid in 'server block' :  \'" + directives["root"] + "\'"), false);
	
	if (directives.find("index") != directives.end() && !isValidIndex(directives["index"]))
		return (Logger("'index' directive is invalid in 'server block' :  \'" + directives["index"] + "\'"), false);
	
	// if (directives.find("methods") != directives.end() && !isValidMethods(directives["methods"]))
	// 	return (Logger("'methods' directive is invalid in 'server block' :  \'" + directives["methods"] + "\'"), false);
	
	if (directives.find("upload_store") != directives.end() && !isValidPath(directives["upload_store"]))
		return (Logger("'upload_store' directive is invalid in 'server block' :  \'" + directives["upload_store"] + "\'"), false);
	
	if (directives.find("redirect") != directives.end() && !isValidRedirect(directives["redirect"]))
		return (Logger("'redirect' directive is invalid in 'server block' :  \'" + directives["redirect"] + "\'"), false);
	
	if (directives.find("autoindex") != directives.end() && !isValidAutoIndex(directives["autoindex"]))
		return (Logger("'autoindex' directive is invalid in 'server block' :  \'" + directives["autoindex"] + "\'"), false);
	
	// if (directives.find("cgi_pass") != directives.end() && !isValidCgiPass(directives["cgi_pass"]))
	// 	return (Logger("'cgi_pass' directive is invalid in 'server block' :  \'" + directives["cgi_pass"] + "\'"), false);
	
	return true;
}

bool				Config::validateLocationBlockDirectives(std::map<std::string, std::string>& directives) {
	if (directives.find("location") == directives.end())
		return (Logger("'location' directive is missing in 'location block'"), false);
	
	if (directives.find("location") != directives.end() && !isValidPath(directives["location"]))
		return (Logger("'location' directive is invalid in 'location block' :  \'" + directives["location"] + "\'"), false);
	
	if (directives.find("error_page") != directives.end() && !isValidErrorPage(directives["error_page"]))
		return (Logger("'error_page' directive is invalid in 'location block' :  \'" + directives["error_page"] + "\'"), false);
	
	if (directives.find("client_max_body_size") != directives.end() && !isValidClientMaxBodySize(directives["client_max_body_size"]))
		return (Logger("'client_max_body_size' directive is invalid in 'location block' :  \'" + directives["client_max_body_size"] + "\'"), false);
	
	if (directives.find("root") != directives.end() && !isValidPath(directives["root"]))
		return (Logger("'root' directive is invalid in 'location block' :  \'" + directives["root"] + "\'"), false);
	
	if (directives.find("alias") != directives.end() && !isValidPath(directives["alias"])) // isValidAlias() ?
		return (Logger("'alias' directive is invalid in 'location block' :  \'" + directives["alias"] + "\'"), false);
	
	if (directives.find("index") != directives.end() && !isValidIndex(directives["index"]))
		return (Logger("'index' directive is invalid in 'server block' :  \'" + directives["index"] + "\'"), false);
	
	if (directives.find("methods") != directives.end() && !isValidMethods(directives["methods"]))
		return (Logger("'methods' directive is invalid in 'location block' :  \'" + directives["methods"] + "\'"), false);
	
	if (directives.find("upload_store") != directives.end() && !isValidPath(directives["upload_store"]))
		return (Logger("'upload_store' directive is invalid in 'location block' :  \'" + directives["upload_store"] + "\'"), false);
	
	if (directives.find("redirect") != directives.end() && !isValidRedirect(directives["redirect"]))
		return (Logger("'redirect' directive is invalid in 'location block' :  \'" + directives["redirect"] + "\'"), false);
	
	if (directives.find("autoindex") != directives.end() && !isValidAutoIndex(directives["autoindex"]))
		return (Logger("'autoindex' directive is invalid in 'location block' :  \'" + directives["autoindex"] + "\'"), false);
	
	if (directives.find("cgi_pass") != directives.end() && !isValidCgiPass(directives["cgi_pass"]))
		return (Logger("'cgi_pass' directive is invalid in 'location block' :  \'" + directives["cgi_pass"] + "\'"), false);
	
	if (directives.find("cgi_ext") != directives.end() && !isValidCgiPass(directives["cgi_ext"])) // isValidCgiExt();
		return (Logger("'cgi_ext' directive is invalid in 'location block' :  \'" + directives["cgi_ext"] + "\'"), false);
	
	return true;
}

bool				Config::isAllowedDirective(const std::string& directive, const std::string& blockType) {
	std::string		allowedDirectives[15] = { 
												"host", "port", "server_name", "error_page", "client_max_body_size",
												"root", "index", "autoindex", "redirect", "upload_store",
												"location", "methods", "alias", "cgi_pass", "cgi_ext"
											};

	if (blockType == "server") {
		for (size_t i = 0; i <= 9; i++) {
			if (directive == allowedDirectives[i])
				return true;
		}
		return false;
	} else if (blockType == "location") {
		for (size_t i = 3; i < 15; i++) {
			if (directive == allowedDirectives[i])
				return true;
		}
		return false;
	}
	return false;

	// if (blockType == "server") {
	// 	if (defaultServerDirectives.find(directive) == defaultServerDirectives.end())
	// 		return false;
	// 	return true;
	// } else if (blockType == "location") {
	// 	if (defaultLocationDirectives.find(directive) == defaultLocationDirectives.end())
	// 		return false;
	// 	return true;
	// }
	// return false;
}

bool				Config::addToServerParserServerDirectives(const std::string& key, const std::string& value, std::map<std::string, std::string>& directives) {
	bool				isPresent = directives.find(key) != directives.end();

	if (isPresent) {
		if (key == "host" || key == "port")
			return false;
		if (key == "server_name")
			return (directives["server_name"] += " " + value, true);
		if (key == "error_page")
			return (directives["error_page"] += " ; " + value, true);
	}
	directives[key] = value;
	return true;
}

bool				Config::addToServerParserLocationDirectives(const std::string& key, const std::string& value, std::map<std::string, std::string>& directives) {
	bool										isPresent = directives.find(key) != directives.end();

	if (isPresent) {
		if (key == "location")
			return false;
		// if (key == "methods")
		// 	return (directives["methods"] += " " + value, true);
	}
	directives[key] = value;
	return true;
}
