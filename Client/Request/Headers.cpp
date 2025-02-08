/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmaila <mmaila@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 18:26:22 by nazouz            #+#    #+#             */
/*   Updated: 2025/02/08 16:14:05 by mmaila           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
//		return (setStatusCode(400), false);

bool			Request::headerExists(const std::string& key) {
	std::map<std::string, std::string>::iterator it;
	it = _RequestData.Headers.find(key);
	if (it == _RequestData.Headers.end())
		return false;
	return true;
}

bool			Request::bufferContainHeaders() {
	size_t			pos;
	
	pos = buffer.find("\r\n\r\n");
	if (pos == std::string::npos)
		return false;
	return true;
}

std::string		Request::extractHeadersFromBuffer() {
	size_t			CRLF;
	std::string		header;
	
	CRLF = buffer.find("\r\n\r\n");
	if (CRLF == std::string::npos)
		return "";
	header = buffer.substr(0, CRLF) + "\r\n\r\n";
	buffer = buffer.erase(0, CRLF + 4);
	bufferSize -= (CRLF + 4);
	return header;
}

bool			Request::decodeURI() {
	std::string			encodedURI = _RequestData.URI;
	std::string			decodedURI;

	// std::cout << "Encoded URI: " << encodedURI << std::endl;
	for (size_t i = 0; i < encodedURI.size(); i++) {
		if (encodedURI[i] == '%' && i + 2 < encodedURI.size() && isHexa(encodedURI.substr(i + 1, 2))) {
			decodedURI += hexToInt(encodedURI.substr(i + 1, 2));
			i += 2;
		} else if (encodedURI[i] == '+') {
			decodedURI += ' ';
		} else {
			decodedURI += encodedURI[i];
		}
	}
	// std::cout << "Decoded URI: " << decodedURI << std::endl;
	_RequestData.URI = decodedURI;
	return true;
}

bool			Request::isValidMethod(const std::string& method) {
	// if (method != "GET" && method != "POST" && method != "DELETE" &&
	// 	method != "HEAD" && method != "PUT" && method != "CONNECT" &&
	// 	method != "OPTIONS" && method != "TRACE" && method != "PATCH")
	// 	return false;
	if (method != "GET" && method != "POST" && method != "DELETE")
	{
		std::cout << "METHOD >>>> " << method << std::endl;
		return (setStatusCode(405), false);
	}
	if (method == "DELETE")
		statusCode = 204;
	return true;
}

bool			Request::isValidURI(const std::string& uri) 
{
	if (uri.size() > 2048)
		return (setStatusCode(414), false);

	if (uri[0] != '/')
	{
		statusCode = 400;
		return (false);
	}
	std::string	allowedURIChars 
		= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ._-~:/?#[]@!$&'()*+,;=%";
	for (size_t i = 0; i < uri.size(); i++)
		if (allowedURIChars.find(uri[i]) == std::string::npos)
			return (setStatusCode(400), false);
	return true;
}

bool			Request::isValidHTTPVersion(const std::string& httpversion) {
	if (httpversion != "HTTP/1.1")
		return (setStatusCode(505), false);
	return true;
}

bool			Request::isValidFieldLine(const std::string& fieldline) {
	std::string			line(fieldline);
	size_t				colonPos;

	colonPos = line.find(':');
	if (colonPos == std::string::npos)
		return (setStatusCode(400), false);
	
	std::string			fieldName = line.substr(0, colonPos);
	if (fieldName.empty() || headerExists(fieldName))
		return (setStatusCode(400), false);
	
	std::string allowedChars 
		= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!#$%&'*+-.^_`|~";
	for (size_t i = 0; i < fieldName.size(); i++)
		if (allowedChars.find(fieldName[i]) == std::string::npos)
			return (setStatusCode(400), false);
	return true;
}

bool			Request::parseHeaders() {
	std::string			fieldname;
	std::string			fieldvalue;

	for (size_t i = 1; i < _RequestRaws.rawHeaders.size(); i++) {
		if (_RequestRaws.rawHeaders[i].empty())
			continue;
		if (!isValidFieldLine(_RequestRaws.rawHeaders[i]))
			return false;
		int colonPos = _RequestRaws.rawHeaders[i].find(':');
		fieldname = stringtolower(_RequestRaws.rawHeaders[i].substr(0, colonPos));
		fieldvalue = stringtrim(_RequestRaws.rawHeaders[i].substr(colonPos + 1), " \t");
		_RequestData.Headers[fieldname] = fieldvalue;
	}

	std::map<std::string, std::string>::iterator it;
	it = _RequestData.Headers.find("host");
	if (it != _RequestData.Headers.end())
		_RequestData.host = it->second;
	
	it = _RequestData.Headers.find("content-type");
	if (it != _RequestData.Headers.end())
		_RequestData.contentType = it->second;
	
	it = _RequestData.Headers.find("connection");
	if (it != _RequestData.Headers.end()) {
		_RequestData.connection = it->second;
		_RequestData.keepAlive = (it->second == "keep-alive" || it->second == "keepAlive");
	}
	
	it = _RequestData.Headers.find("transfer-encoding");
	if (it != _RequestData.Headers.end())
		_RequestData.transferEncoding = stringtolower(it->second);
	
	it = _RequestData.Headers.find("content-length");
	if (it != _RequestData.Headers.end() && stringIsDigit(it->second))
		_RequestData.contentLength = std::atoi(it->second.c_str());
	
	return true;
}

bool			Request::parseRequestLine() {
	std::string			token;
	std::string			tokens[3];
	size_t				tokenCount = 0;
	size_t				spaceCount = 0;
	std::stringstream	ss(_RequestRaws.rawHeaders[0]);

	_RequestRaws.rawRequestLine = _RequestRaws.rawHeaders[0];
	for (size_t i = 0; _RequestRaws.rawRequestLine[i]; i++)
		if (_RequestRaws.rawRequestLine[i] == ' ')
			spaceCount++;
	if (spaceCount != 2)
		return (setStatusCode(400), false);
	while (ss >> token && tokenCount < 3) { // bug here! this allows tabs which goes against HTTP/1.1 RFC
		tokens[tokenCount++] = token;
		token = "";
	}
	if (!token.empty())
		return (setStatusCode(400), false);
	

	_RequestData.Method = tokens[0];
	_RequestData.URI = tokens[1];
	_RequestData.HTTPversion = tokens[2];
	return     isValidMethod(_RequestData.Method)
			&& isValidURI(_RequestData.URI)
			&& isValidHTTPVersion(_RequestData.HTTPversion)
			&& decodeURI();
}

bool			Request::storeHeadersInVector() {
	size_t						rpos;
	std::string					line;
	std::string					toParse;
	size_t						opos = 0;
	
	toParse = extractHeadersFromBuffer();
	if (toParse.empty())
		return (setStatusCode(500), false);
	while (toParse[opos]) {
		rpos = toParse.find("\r\n", opos);
		if (rpos == std::string::npos)
			return (setStatusCode(400), false);
		line = toParse.substr(opos, rpos - opos);
		if (line.empty())
			break ;
		_RequestRaws.rawHeaders.push_back(line);
		opos = rpos + 2;
	}
	return true;
}

// this function should be revised against RFC
bool			Request::validateRequestHeaders() {
	return (true);
	if (!headerExists("host") || _RequestData.host.empty())
		return (setStatusCode(400), false);
	
	if (!headerExists("connection") || _RequestData.connection.empty())
		return (setStatusCode(400), false);
	else
		_RequestData.Headers.insert(std::make_pair("connection", "keep-alive"));
	
	if (_RequestData.Method == "POST") {
		bool			ContentLength = headerExists("content-length");
		bool			TransferEncoding = headerExists("transfer-encoding");

		if (ContentLength == TransferEncoding)
			return (setStatusCode(400), false);
		
		if (TransferEncoding && _RequestData.transferEncoding == "chunked")
			isEncoded = true;
		else if (TransferEncoding && _RequestData.transferEncoding != "chunked")
			return (setStatusCode(501), false);

		if (ContentLength && _RequestRaws.contentLength == -1) // && body.contentLength == -1 ???
			return (setStatusCode(400), false);
		

		int pos = _RequestData.contentType.find("multipart/form-data; boundary=") + 30;
		if (headerExists("content-type") && pos != 30)
			return (setStatusCode(501), false);
		else if (headerExists("content-type") && pos == 30 && _RequestData.contentType[pos])
			_RequestRaws.boundaryBegin = "--" + _RequestData.contentType.substr(pos), _RequestRaws.boundaryEnd = _RequestRaws.boundaryBegin + "--", isMultipart = true;
	}
	return true;
}

// HEADERS CONTROL CENTER
bool			Request::parseRequestLineAndHeaders() {
	if (!storeHeadersInVector())
		std::cerr << "[ERROR]\tstoreHeadersInVector();" << std::endl;
	if (!parseRequestLine())
		std::cerr << "[ERROR]\tparseRequestLine();" << std::endl;
	if (!parseHeaders())
		std::cerr << "[ERROR]\tparseHeaders();" << std::endl;
	if (!validateRequestHeaders())
		std::cerr << "[ERROR]\tvalidateRequestHeaders();" << std::endl;
	setMatchingConfig();
	fillRequestData(_RequestData.URI, _RequestData);
	std::cout << "status code : " << statusCode << std::endl;
	if (statusCode == 200)
		pState = HEADERS_FINISHED;
	else
		pState = PARSING_FINISHED;
	return true;
}
