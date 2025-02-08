/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Helpers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmaila <mmaila@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 16:50:46 by nazouz            #+#    #+#             */
/*   Updated: 2025/02/08 16:14:27 by mmaila           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientHandler.hpp"

// void						decodeURL(std::string URL) {
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
// 			std::vector<std::string>::iterator	it = _RequestData._Config->index.begin();
			
// 			for (; it != _RequestData._Config->index.end(); it++) {
// 				if (access((_RequestData.fullPath + (*it)).c_str(), F_OK) == 0) {
// 					_RequestData.fullPath += *it;
// 					_RequestData.isDir = false;
// 					break ;
// 				}
// 			}
			
// 			if (it == _RequestData._Config->index.end()) {
// 				_RequestData.StatusCode = _RequestData._Config->autoindex ? _RequestData.StatusCode : 404;
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
// 			std::map<std::string, std::string>::iterator it = _RequestData._Config->cgi_ext.find(fileName.substr(pos));
			
// 		}
// 	}
// }

void						resolveRootAlias(std::string& requestedResource, RequestData& _RequestData) {
	
	if (!_RequestData._Config->alias.empty()) {
		if (_RequestData._Config->alias[_RequestData._Config->alias.length() - 1] == '/')
			_RequestData._Config->alias.erase(_RequestData._Config->alias.length() - 1);
		requestedResource = _RequestData._Config->alias + _RequestData.URI.substr(_RequestData.matchingLocation.length());
	}
	else {
		if (_RequestData._Config->root[_RequestData._Config->root.length() - 1] == '/')
			_RequestData._Config->root.erase(_RequestData._Config->root.length() - 1);
		requestedResource = _RequestData._Config->root + _RequestData.URI;
	}
}

void						setQueryString(std::string& requestedResource, RequestData& _RequestData) {
	size_t			pos;

	pos = requestedResource.find_last_of('#');
	if (pos != std::string::npos)
		requestedResource.erase(pos);
	pos = requestedResource.find_last_of('?'); // should we check for the first or the last??
	if (pos != std::string::npos)
		_RequestData.queryString = requestedResource.substr(pos + 1), requestedResource.erase(pos);
}

void						setRequestedResourceType(std::string& requestedResource, RequestData& _RequestData) {
	size_t			pos;
	struct stat		pathStats;

	while (!requestedResource.empty()) {
		pos = requestedResource.find('/', 1);
		_RequestData.fullPath += requestedResource.substr(0, pos);
		requestedResource.erase(0, pos);
		if (stat(_RequestData.fullPath.c_str(), &pathStats) != 0) 
			throw(404);
		if (!S_ISDIR(pathStats.st_mode))
			break ;
	}

	_RequestData.isDir = S_ISDIR(pathStats.st_mode) ? true : false;
	
	std::cout << BLUE << "FULL_PATH AFTER ISFILE ISDIR: " << _RequestData.fullPath << RESET << std::endl;
}

void						handleDirectoryResource(RequestData& _RequestData) {
	if (access(_RequestData.fullPath.c_str(), X_OK) != 0)
		throw(403);
	
	if (_RequestData.Method == "GET") {
		std::vector<std::string>::iterator	it = _RequestData._Config->index.begin();
		
		for (; it != _RequestData._Config->index.end(); it++) {
			if (access((_RequestData.fullPath + (*it)).c_str(), F_OK) == 0) {
				_RequestData.fullPath += *it;
				_RequestData.isDir = false;
				break ;
			}
		}
		
		if (it == _RequestData._Config->index.end()) {
			if (!_RequestData._Config->autoindex)
				throw(404);
		}
	}
	
	std::cout << BLUE << "FULL_PATH AFTER ISDIR: " << _RequestData.fullPath << RESET << std::endl;
}

bool						extensionIsCGI(const std::string& extension, RequestData& _RequestData) {
	if (extension.empty())
		return false;
	
	std::map<std::string, std::string>::iterator it = _RequestData._Config->cgi_ext.find(extension);
	if (it != _RequestData._Config->cgi_ext.end())
	{
		_RequestData.cgiIntrepreter = it->second;
		return true;
	}
	return false;
}

void						handleFileResource(const std::string& path_info, RequestData& _RequestData) {
	if (access(_RequestData.fullPath.c_str(), R_OK) != 0)
		throw(403);
	
	std::string filename = _RequestData.fullPath.substr(_RequestData.fullPath.find_last_of('/') + 1);
	size_t		pos		 = filename.find_last_of('.');
	std::string file_ext = (pos == std::string::npos) ? "" : filename.substr(pos);
	
	_RequestData.isCGI = extensionIsCGI(file_ext, _RequestData);
	if (_RequestData.isCGI) {
		_RequestData.pathInfo = path_info;
		_RequestData.scriptName = filename;
		// setupCGI();
	}
	else if (!path_info.empty())
		throw(404);
}


void			decodeAbsPath(const std::string absPath, RequestData& _RequestData) {
	_RequestData.fullPath.clear();
	
	std::string	requestedResource = absPath;

	setQueryString(requestedResource, _RequestData);
	setRequestedResourceType(requestedResource, _RequestData);
	if (_RequestData.isDir)
		handleDirectoryResource(_RequestData);
	if (!_RequestData.isDir)
		handleFileResource(requestedResource, _RequestData);
}

void			fillRequestData(const std::string URI, RequestData& _RequestData) {
	_RequestData.fullPath.clear();
	_RequestData.URI = URI;
	
	if (!rootJail(URI))
		throw(403);

	std::string				requestedResource;

	resolveRootAlias(requestedResource, _RequestData);
	setQueryString(requestedResource, _RequestData);
	setRequestedResourceType(requestedResource, _RequestData);
	if (_RequestData.isDir)
		handleDirectoryResource(_RequestData);
	if (!_RequestData.isDir)
		handleFileResource(requestedResource, _RequestData);
	// std::cout << "FULL PATH AFTER ALL : " << _RequestData.fullPath << std::endl;
	// std::cout << "CGI : " << _RequestData.isCGI << std::endl;
}
