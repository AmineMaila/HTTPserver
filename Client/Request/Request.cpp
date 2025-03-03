/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nazouz <nazouz@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/21 10:33:50 by nazouz            #+#    #+#             */
/*   Updated: 2025/03/01 18:42:35 by nazouz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::~Request() {}

Request::Request(std::vector<ServerConfig>&	vServers) : vServers(vServers) {
	bufferSize = 0;
	
	isEncoded = false;
	isMultipart = false;
	headersFinished = false;
	bodyFinished = false;
	
	_RequestData._Config = &vServers[0].ServerDirectives;

	_RequestRaws.mimeTypes["text/html"] = ".html";
	_RequestRaws.mimeTypes["text/css"] = ".css";
	_RequestRaws.mimeTypes["text/csv"] = ".csv";
	_RequestRaws.mimeTypes["application/doc"] = ".doc";
	_RequestRaws.mimeTypes["image/gif"] = ".gif";
	_RequestRaws.mimeTypes["image/jpeg"] = ".jpg";
	_RequestRaws.mimeTypes["text/javascript"] = ".js";
	_RequestRaws.mimeTypes["application/json"] = ".json";
	_RequestRaws.mimeTypes["application/java-archive"] = ".jar";
	_RequestRaws.mimeTypes["audio/mpeg"] = ".mp3";
	_RequestRaws.mimeTypes["video/mp4"] = ".mp4";
	_RequestRaws.mimeTypes["video/mpeg"] = ".mpeg";
	_RequestRaws.mimeTypes["image/png"] = ".png";
	_RequestRaws.mimeTypes["application/pdf"] = ".pdf";
	_RequestRaws.mimeTypes["application/x-sh"] = ".sh";
	_RequestRaws.mimeTypes["audio/wav"] = ".wav";
	_RequestRaws.mimeTypes["audio/webm"] = ".weba";
	_RequestRaws.mimeTypes["video/webm"] = ".webm";
	_RequestRaws.mimeTypes["image/webp"] = ".webp";
	_RequestRaws.mimeTypes["application/xml"] = ".xml";
	_RequestRaws.mimeTypes["application/zip"] = ".zip";
	_RequestRaws.mimeTypes["application/x-tar"] = ".tar";
	_RequestRaws.mimeTypes["application/octet-stream"] = ".bin";
	_RequestRaws.mimeTypes["image/avif"] = ".avif";
	_RequestRaws.mimeTypes["video/x-msvideo"] = ".avi";
}

Request::Request(const Request& rhs) : vServers(rhs.vServers) {
	*this = rhs;
}

Request&	Request::operator=(const Request& rhs)
{
	if (this != &rhs)
	{
		buffer = rhs.buffer;
		bufferSize = rhs.bufferSize;

		_RequestData = rhs._RequestData;
		_RequestRaws = rhs._RequestRaws;
		
		headersFinished = rhs.headersFinished;
		bodyFinished = rhs.bodyFinished;
		isEncoded = rhs.isEncoded;
		isMultipart = rhs.isMultipart;
	}
	return *this;
}

// RequestData::RequestData() : isCGI(false), isDir(false), isRange(false), keepAlive(true), StatusCode(200) {
// 	// should i generate cgitempfilename here? we will see
// 	contentLength = 0;
// 	_Config = NULL; // ?
// }

// RequestData&		RequestData::operator=(const RequestData &rhs) {
// 	if (this != &rhs) {
// 		isCGI = rhs.isCGI;
// 		isDir = rhs.isDir;
// 		isRange = rhs.isRange;
// 		keepAlive = rhs.keepAlive;
// 		StatusCode = rhs.StatusCode;

// 		Method = rhs.Method;
// 		URI = rhs.URI;
// 		HTTPversion = rhs.HTTPversion;

// 		queryString = rhs.queryString;
// 		pathInfo = rhs.pathInfo;
// 		scriptName = rhs.scriptName;
// 		cgiIntrepreter = rhs.cgiIntrepreter;
// 		CGITempFilename = rhs.CGITempFilename;
		
// 		fullPath = rhs.fullPath;
// 		contentType = rhs.contentType;
// 		contentLength = rhs.contentLength;
// 		connection = rhs.connection;
// 		host = rhs.host;
// 		transferEncoding = rhs.transferEncoding;
// 		matchingLocation = rhs.matchingLocation;

// 		Headers = rhs.Headers;

// 		// _Config: deep copy or shallow copy?
// 		// if (rhs._Config) {
// 		// 	if (!_Config) {
// 		// 		_Config = new Directives(*rhs._Config); // Deep copy
// 		// 	} else {
// 		// 		*_Config = *rhs._Config;
// 		// 	}
// 		// } else {
// 		// 	delete _Config;
// 		// 	_Config = nullptr;
// 		// }

// 		// reset CGITempFilestream since fstream cannot be copied
// 		if (CGITempFilestream.is_open())
// 			CGITempFilestream.close();
// 		// if (!rhs.CGITempFilename.empty()) {
// 		// 	CGITempFilestream.open(rhs.CGITempFilename, std::ios::in | std::ios::out);
// 		// }
// 	}
// 	return *this;
// }
