/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   URI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmaila <mmaila@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 20:11:27 by nazouz            #+#    #+#             */
/*   Updated: 2025/03/08 19:32:04 by mmaila           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientHandler.hpp"

void	produceAbsPath(RequestData& requestData)
{
	
	if (!requestData.config->redirect.second.empty())
	{
		if (requestData.config->redirect.second.find("http://") != std::string::npos
			|| requestData.config->redirect.second.find("https://") != std::string::npos)
		{
			throw(Code(requestData.config->redirect.first, requestData.config->redirect.second));
		}
		else
		{
			if (requestData.config->redirect.second.at(0) != '/')
				requestData.config->redirect.second.insert(0, "/");
			throw(Code(requestData.config->redirect.first, "http://" + requestData.host + requestData.config->redirect.second));
		}
	}

	if (!requestData.config->alias.empty())
	{
		if (requestData.config->alias[requestData.config->alias.length() - 1] == '/')
			requestData.config->alias.erase(requestData.config->alias.length() - 1);
		requestData.fullPath = requestData.config->alias + requestData.URI.substr(requestData.matchingLocation.length());
	}
	else
	{
		if (requestData.config->root[requestData.config->root.length() - 1] == '/')
			requestData.config->root.erase(requestData.config->root.length() - 1);
		requestData.fullPath = requestData.config->root + requestData.URI;
	}
}

void	setQueryString(RequestData& requestData)
{
	size_t		pos;
	std::string	query;

	pos = requestData.fullPath.find_last_of('#');
	if (pos != std::string::npos)
		requestData.fullPath.erase(pos);
	
	pos = requestData.fullPath.find_first_of('?');
	if (pos != std::string::npos)
	{
		requestData.queryString = requestData.fullPath.substr(pos + 1);
		requestData.fullPath.erase(pos);
	}
}

void	setRequestedResourceType(RequestData& requestData)
{
	struct stat	pathStats;
	std::string	pathChecker;
	
	size_t	startPos = 0;
	size_t	endPos = 0;
	while (startPos < requestData.fullPath.size())
	{
		endPos = requestData.fullPath.find('/', startPos + 1);
		pathChecker.append(requestData.fullPath.substr(startPos, endPos - startPos));
		if (stat(pathChecker.c_str(), &pathStats) != 0)
			throw(Code(404));
		if (!S_ISDIR(pathStats.st_mode))
			break ;
		startPos = endPos;
	}
	requestData.isDir = S_ISDIR(pathStats.st_mode) ? true : false;
	std::string pathInfo = requestData.fullPath.substr(pathChecker.size());
	if (!pathInfo.empty())
		requestData.scriptName = requestData.URI.substr(0, requestData.URI.find(pathInfo));
	else
		requestData.scriptName = requestData.URI.substr(0, requestData.URI.find_first_of("?"));
	requestData.pathTranslated = requestData.fullPath;
	requestData.pathInfo = requestData.URI.substr(0, requestData.URI.find_first_of('?'));
	requestData.fullPath.erase(pathChecker.size());
}

void	handleDirectoryResource(RequestData& requestData)
{
	if (access(requestData.fullPath.c_str(), X_OK) != 0)
		throw(Code(403));
	
	if (requestData.fullPath.at(requestData.fullPath.size() - 1) != '/')
		requestData.fullPath.append("/");

	if (requestData.Method == "GET")
	{
		std::vector<std::string>::iterator	it = requestData.config->index.begin();
		
		for (; it != requestData.config->index.end(); it++)
		{
			if (it->at(0) == '/')
				it->erase(0, 1);
			if (access((requestData.fullPath + (*it)).c_str(), F_OK) == 0)
			{
				requestData.fullPath += *it;
				requestData.isDir = false;
				break ;
			}
		}
		
		if (it == requestData.config->index.end())
		{
			if (!requestData.config->autoindex)
				throw(Code(403));
		}
	}
}

bool	extensionIsCGI(RequestData& requestData)
{
	size_t dot = requestData.fullPath.find_last_of('.');
	if (dot == std::string::npos)
		return (false);

	std::string file_ext = requestData.fullPath.substr(dot);
	
	std::map<std::string, std::string>::iterator it = requestData.config->cgi_ext.find(file_ext);
	if (it != requestData.config->cgi_ext.end())
	{
		requestData.cgiIntrepreter = it->second;
		return true;
	}
	return false;
}

void	handleFileResource(RequestData& requestData)
{
	if (access(requestData.fullPath.c_str(), R_OK) != 0)
		throw(Code(403));

	requestData.fileName = requestData.fullPath.substr(requestData.fullPath.find_last_of('/') + 1);

	requestData.isCGI = extensionIsCGI(requestData);
	if (!requestData.isCGI && requestData.scriptName != requestData.pathInfo)
		throw(Code(404));
}

void	resolveAbsPath(RequestData& requestData)
{

	setQueryString(requestData);
	setRequestedResourceType(requestData);
	if (requestData.isDir)
		handleDirectoryResource(requestData);
	if (!requestData.isDir)
		handleFileResource(requestData);
}

void	resolveURI(RequestData& requestData)
{
	requestData.fullPath.clear();

	produceAbsPath(requestData);
	resolveAbsPath(requestData);
	if (requestData.isDir && requestData.URI[requestData.URI.size() - 1] != '/')
		throw(Code(308, "http://" + requestData.host + requestData.URI + '/'));

	if (requestData.Method == "POST")
	{
		if (!requestData.isCGI)
		{
			if (requestData.config->upload_store.empty())
				throw (Code(403));
			else if (!requestData.isDir)
				throw (Code(405));
		}
		if (requestData.contentLength > requestData.config->client_max_body_size)
			throw (Code(413));
	}
}
