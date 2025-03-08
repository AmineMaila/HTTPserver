/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmaila <mmaila@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/21 10:33:50 by nazouz            #+#    #+#             */
/*   Updated: 2025/03/08 19:32:04 by mmaila           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request(std::vector<ServerConfig>&	vServers) : vServers(vServers)
{
	headersParsed = false;
	bodyStored = false;
	
	requestData.config = &vServers[0].ServerDirectives;
	
	requestRaws.mimeTypes["text/plain"] = "";
	requestRaws.mimeTypes["text/html"] = ".html";
	requestRaws.mimeTypes["text/css"] = ".css";
	requestRaws.mimeTypes["text/csv"] = ".csv";
	requestRaws.mimeTypes["application/doc"] = ".doc";
	requestRaws.mimeTypes["image/gif"] = ".gif";
	requestRaws.mimeTypes["image/jpeg"] = ".jpg";
	requestRaws.mimeTypes["text/javascript"] = ".js";
	requestRaws.mimeTypes["application/json"] = ".json";
	requestRaws.mimeTypes["application/java-archive"] = ".jar";
	requestRaws.mimeTypes["audio/mpeg"] = ".mp3";
	requestRaws.mimeTypes["video/mp4"] = ".mp4";
	requestRaws.mimeTypes["video/mpeg"] = ".mpeg";
	requestRaws.mimeTypes["image/png"] = ".png";
	requestRaws.mimeTypes["application/pdf"] = ".pdf";
	requestRaws.mimeTypes["application/x-sh"] = ".sh";
	requestRaws.mimeTypes["audio/wav"] = ".wav";
	requestRaws.mimeTypes["audio/webm"] = ".weba";
	requestRaws.mimeTypes["video/webm"] = ".webm";
	requestRaws.mimeTypes["image/webp"] = ".webp";
	requestRaws.mimeTypes["application/xml"] = ".xml";
	requestRaws.mimeTypes["application/zip"] = ".zip";
	requestRaws.mimeTypes["application/x-tar"] = ".tar";
	requestRaws.mimeTypes["application/octet-stream"] = ".bin";
	requestRaws.mimeTypes["image/avif"] = ".avif";
	requestRaws.mimeTypes["video/x-msvideo"] = ".avi";
}

Request::Request(const Request& rhs) : vServers(rhs.vServers)
{
	*this = rhs;
}

Request&	Request::operator=(const Request& rhs)
{
	if (this != &rhs)
	{
		buffer = rhs.buffer;
		
		requestData = rhs.requestData;
		requestRaws = rhs.requestRaws;
		
		headersParsed = rhs.headersParsed;
		bodyStored = rhs.bodyStored;
	}
	return *this;
}

Request::~Request()
{
	if (fileUploader.is_open())
		fileUploader.close();
}

#include "Request.hpp"

ServerConfig&	Request::getMatchingServer()
{
	for (size_t i = 0; i < vServers.size(); i++)
	{
		for (size_t j = 0; j < vServers[i].server_names.size(); j++)
		{
			if (vServers[i].server_names[j] == requestData.serverHost)
				return vServers[i];
		}
	}
	return vServers[0];
}

void	Request::setMatchingConfig()
{
	ServerConfig&	matchingServer = getMatchingServer();
	
	std::map<std::string, Directives>::iterator it = matchingServer.Locations.begin();
	for (; it != matchingServer.Locations.end(); it++)
	{
		if (requestData.URI.find(it->first) == 0)
		{
			if (it->first.size() > requestData.matchingLocation.size())
				requestData.matchingLocation = it->first;
		}
	}

	if (requestData.matchingLocation.empty())
		requestData.config = &matchingServer.ServerDirectives;
	else
		requestData.config = &matchingServer.Locations.find(requestData.matchingLocation)->second;
	
	if (std::find(requestData.config->methods.begin(), requestData.config->methods.end(), requestData.Method) == requestData.config->methods.end())
		throw Code(405);
}

// PARSING CONTROL CENTER
int	Request::process(char *recvBuffer, int recvBufferSize)
{
	buffer.append(recvBuffer, recvBufferSize);

	if (!headersParsed)
	{
		if (buffer.size() > 16384) // 16KB
			throw Code(431);
		if (buffer.find(DOUBLE_CRLF) == std::string::npos)
			return RECV;

		parseRequestLine();
		parseHeaders();
		validateHeaders();
		setMatchingConfig();
		resolveURI(requestData);
		headersParsed = true;
		if (requestData.Method != "POST")
			return RESPOND;
		if (requestData.isCGI)
		{
			if (!requestData.isEncoded)
				return FORWARD_CGI;
			openTmpFile();
		}
	}
	processBody();
	return bodyStored;
}