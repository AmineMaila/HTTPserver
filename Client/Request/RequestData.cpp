/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestData.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nazouz <nazouz@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 17:46:55 by nazouz            #+#    #+#             */
/*   Updated: 2025/02/05 17:16:18 by nazouz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

ServerConfig&				Request::getMatchingServer() {
	for (size_t i = 0; i < vServers.size(); i++) {
		for (size_t j = 0; j < vServers[i].server_names.size(); j++) {
			if (vServers[i].server_names[j] == _RequestData.host)
				return vServers[i];
		}
	}
	return vServers[0];
}

void						Request::setMatchingConfig() {
	ServerConfig&			matchingServer = getMatchingServer();
	
	// this is implemented by mmaila, i should check if its working properly with prefix matching
	std::map<std::string, Directives>::iterator it = matchingServer.Locations.begin();
	for (; it != matchingServer.Locations.end(); it++) {
		if (_RequestData.URI.find(it->first) != std::string::npos) {
			if (it->first.size() > _RequestData.matchingLocation.size())
				_RequestData.matchingLocation = it->first;
		}
	}

	_RequestData._Config = (_RequestData.matchingLocation.empty()) ? 
	&matchingServer.ServerDirectives : &matchingServer.Locations.find(_RequestData.matchingLocation)->second;
}






























































// void						Request::decodeURL(std::string URL) {
// 	if (!rootJail(_RequestData.URI)) {
// 		_RequestData.StatusCode = 403;
// 		return ;
// 	}
	
// 	size_t			pos;
// 	struct stat		pathStats;

// 	pos = URL.find_last_of('#');
// 	if (pos != std::string::npos)
// 		URL.erase(pos);
// 	pos = URL.find_last_of('?'); // should we check for the first or the last??
// 	if (pos != std::string::npos)
// 		_RequestData.QUERYSTRING = URL.substr(pos + 1), URL.erase(pos);
	
// 	while (!URL.empty()) {
// 		pos = URL.find('/', 1);
// 		_RequestData.fullPath += URL.substr(0, pos);
// 		if (stat(_RequestData.fullPath.c_str(), &pathStats) != 0) {
// 			_RequestData.StatusCode = 404;
// 			return ;
// 		}
// 		if (!S_ISDIR(pathStats.st_mode))
// 			break ;
// 	}
// 	// when breaking from this loop; either fullPath is existing Dir or File
// 	// so if it is a Dir => append indexes => file
// 	// and then => check if the file extension is a cgi supported extension
// 	std::cout << BLUE << "FULL_PATH AFTER ISFILE ISDIR: " << _RequestData.fullPath << RESET << std::endl;

// 	_RequestData.isDir = S_ISDIR(pathStats.st_mode) ? true : false;
	
// 	// if isDir append indexes
// 	if (_RequestData.isDir) {
// 		if (access(_RequestData.fullPath.c_str(), X_OK) != 0) {
// 			_RequestData.StatusCode = 403;
// 			return ;
// 		}
		
// 		if (_RequestData.Method == "GET") {
// 			std::vector<std::string>::iterator	it = _RequestData._Config.index.begin();
			
// 			for (; it != _RequestData._Config.index.end(); it++) {
// 				if (access((_RequestData.fullPath + (*it)).c_str(), F_OK) == 0) {
// 					_RequestData.fullPath += *it;
// 					_RequestData.isDir = false;
// 					break ;
// 				}
// 			}
			
// 			if (it == _RequestData._Config.index.end()) {
// 				_RequestData.StatusCode = _RequestData._Config.autoindex ? _RequestData.StatusCode : 404;
// 				return ;
// 			}
// 		}
// 		std::cout << BLUE << "FULL_PATH AFTER ISDIR: " << _RequestData.fullPath << RESET << std::endl;
// 	}

// 	if (!_RequestData.isDir) {
// 		if (access(_RequestData.fullPath.c_str(), R_OK) != 0) {
// 			_RequestData.StatusCode = 403;
// 			return ;
// 		}

// 		std::string		fileName = _RequestData.fullPath.substr(_RequestData.fullPath.find_last_of('/') + 1);
// 		if ((pos = fileName.find('.')) != std::string::npos) {
// 			std::map<std::string, std::string>::iterator it = _RequestData._Config.cgi_ext.find(fileName.substr(pos));
			
// 		}
// 	}
// }

// void						Request::resolveRootAlias(std::string& requestedResource) {
// 	std::string				matchingLocation;
	
// 	setMatchingConfig(matchingLocation);
	
// 	if (!_RequestData._Config.alias.empty()) {
// 		if (_RequestData._Config.alias[_RequestData._Config.alias.length() - 1] == '/')
// 			_RequestData._Config.alias.erase(_RequestData._Config.alias.length() - 1);
// 		requestedResource = _RequestData._Config.alias + _RequestData.URI.substr(matchingLocation.length());
// 	}
// 	else {
// 		if (_RequestData._Config.root[_RequestData._Config.root.length() - 1] == '/')
// 			_RequestData._Config.root.erase(_RequestData._Config.root.length() - 1);
// 		requestedResource = _RequestData._Config.root + _RequestData.URI;
// 	}
// }

// void						Request::setQueryString(std::string& requestedResource) {
// 	size_t			pos;

// 	pos = requestedResource.find_last_of('#');
// 	if (pos != std::string::npos)
// 		requestedResource.erase(pos);
// 	pos = requestedResource.find_last_of('?'); // should we check for the first or the last??
// 	if (pos != std::string::npos)
// 		_RequestData.QUERYSTRING = requestedResource.substr(pos + 1), requestedResource.erase(pos);
// }

// void						Request::setRequestedResourceType(std::string& requestedResource) {
// 	size_t			pos;
// 	struct stat		pathStats;

// 	while (!requestedResource.empty()) {
// 		pos = requestedResource.find('/', 1);
// 		_RequestData.fullPath += requestedResource.substr(0, pos);
// 		if (stat(_RequestData.fullPath.c_str(), &pathStats) != 0) {
// 			_RequestData.StatusCode = 404;
// 			return ;
// 		}
// 		if (!S_ISDIR(pathStats.st_mode))
// 			break ;
// 	}

// 	_RequestData.isDir = S_ISDIR(pathStats.st_mode) ? true : false;
	
// 	std::cout << BLUE << "FULL_PATH AFTER ISFILE ISDIR: " << _RequestData.fullPath << RESET << std::endl;
// }

// void						Request::handleDirectoryResource() {
// 	if (access(_RequestData.fullPath.c_str(), X_OK) != 0) {
// 		_RequestData.StatusCode = 403;
// 		return ;
// 	}
	
// 	if (_RequestData.Method == "GET") {
// 		std::vector<std::string>::iterator	it = _RequestData._Config.index.begin();
		
// 		for (; it != _RequestData._Config.index.end(); it++) {
// 			if (access((_RequestData.fullPath + (*it)).c_str(), F_OK) == 0) {
// 				_RequestData.fullPath += *it;
// 				_RequestData.isDir = false;
// 				break ;
// 			}
// 		}
		
// 		if (it == _RequestData._Config.index.end()) {
// 			_RequestData.StatusCode = _RequestData._Config.autoindex ? _RequestData.StatusCode : 404;
// 			return ;
// 		}
// 	}
	
// 	std::cout << BLUE << "FULL_PATH AFTER ISDIR: " << _RequestData.fullPath << RESET << std::endl;
// }

// bool						Request::extensionIsCGI(const std::string& extension) {
// 	if (extension.empty())
// 		return false;
	
// 	std::map<std::string, std::string>::iterator it = _RequestData._Config.cgi_ext.find(extension);
// 	if (it != _RequestData._Config.cgi_ext.end())
// 		return true;
// 	return false;
// }

// void						Request::handleFileResource(const std::string& path_info) {
// 	if (access(_RequestData.fullPath.c_str(), R_OK) != 0) {
// 		_RequestData.StatusCode = 403;
// 		return ;
// 	}
	
// 	std::string filename = _RequestData.fullPath.substr(_RequestData.fullPath.find_last_of('/') + 1);
// 	size_t		pos		 = filename.find_last_of('.');
// 	std::string file_ext = (pos == std::string::npos) ? "" : filename.substr(pos);
	
// 	_RequestData.isCGI = extensionIsCGI(file_ext);
// 	if (_RequestData.isCGI) {
// 		_RequestData.PATH_INFO = path_info;
// 		_RequestData.SCRIPT_NAME = filename;
// 		// setupCGI();
// 	}
// 	else
// 		_RequestData.StatusCode = (path_info.empty()) ? _RequestData.StatusCode : 404;
// }

// void						Request::processRequestData() {
// 	std::string				requestedResource;
	
// 	if (!rootJail(_RequestData.URI)) {
// 		_RequestData.StatusCode = 403;
// 		return ;
// 	}
	
// 	resolveRootAlias(requestedResource);
// 	setQueryString(requestedResource);
// 	setRequestedResourceType(requestedResource);
// 	if (_RequestData.isDir)
// 		handleDirectoryResource();
// 	if (!_RequestData.isDir)
// 		handleFileResource(requestedResource);
// }
