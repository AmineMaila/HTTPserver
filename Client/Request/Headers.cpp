/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmaila <mmaila@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 18:26:22 by nazouz            #+#    #+#             */
/*   Updated: 2025/02/01 16:04:51 by mmaila           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
//		return (setStatusCode(400), false);

bool			Request::headerExists(const std::string& key) {
	std::map<std::string, std::string>::iterator it;
	it = header.headersMap.find(key);
	if (it == header.headersMap.end())
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
	std::string			encodedURI = requestLine.uri;
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
	requestLine.uri = decodedURI;
	return true;
}

bool			Request::isValidMethod(const std::string& method) {
	// if (method != "GET" && method != "POST" && method != "DELETE" &&
	// 	method != "HEAD" && method != "PUT" && method != "CONNECT" &&
	// 	method != "OPTIONS" && method != "TRACE" && method != "PATCH")
	// 	return false;
	if (method != "GET" && method != "POST" && method != "DELETE")
		return (setStatusCode(405), false);
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

	for (size_t i = 1; i < header.rawHeaders.size(); i++) {
		if (header.rawHeaders[i].empty())
			continue;
		if (!isValidFieldLine(header.rawHeaders[i]))
			return false;
		int colonPos = header.rawHeaders[i].find(':');
		fieldname = header.rawHeaders[i].substr(0, colonPos);
		fieldvalue = stringtrim(header.rawHeaders[i].substr(colonPos + 1), " \t");
		header.headersMap[fieldname] = fieldvalue;
	}

	std::map<std::string, std::string>::iterator it;
	it = header.headersMap.find("Host");
	if (it != header.headersMap.end())
		header.host = it->second;
	
	it = header.headersMap.find("Content-Type");
	if (it != header.headersMap.end())
		header.contentType = it->second;
	
	it = header.headersMap.find("Connection");
	if (it != header.headersMap.end())
		header.connection = it->second;
	
	it = header.headersMap.find("Transfer-Encoding");
	if (it != header.headersMap.end())
		header.transferEncoding = stringtolower(it->second);
	
	it = header.headersMap.find("Content-Length");
	if (it != header.headersMap.end() && stringIsDigit(it->second))
		body.contentLength = std::atoi(it->second.c_str());
	
	return true;
}

bool			Request::parseRequestLine() {
	std::string			token;
	std::string			tokens[3];
	size_t				tokenCount = 0;
	size_t				spaceCount = 0;
	std::stringstream	ss(header.rawHeaders[0]);

	requestLine.rawRequestLine = header.rawHeaders[0];
	for (size_t i = 0; requestLine.rawRequestLine[i]; i++)
		if (requestLine.rawRequestLine[i] == ' ')
			spaceCount++;
	if (spaceCount != 2)
		return (setStatusCode(400), false);
	while (ss >> token && tokenCount < 3) { // bug here! this allows tabs which goes against HTTP/1.1 RFC
		tokens[tokenCount++] = token;
		token = "";
	}
	if (!token.empty())
		return (setStatusCode(400), false);
	

	requestLine.method = tokens[0];
	requestLine.uri = tokens[1];
	requestLine.httpversion = tokens[2];
	return     isValidMethod(requestLine.method)
			&& isValidURI(requestLine.uri)
			&& isValidHTTPVersion(requestLine.httpversion)
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
		header.rawHeaders.push_back(line);
		opos = rpos + 2;
	}
	return true;
}

bool			Request::validateRequestHeaders() {
	return true; // ???
	bool			ContentLength = headerExists("Content-Length");
	bool			TransferEncoding = headerExists("Transfer-Encoding");

	if (ContentLength == TransferEncoding)
		return (setStatusCode(400), false);
	
	if (TransferEncoding && header.transferEncoding == "chunked")
		isEncoded = true;
	else if (TransferEncoding && header.transferEncoding != "chunked")
		return (setStatusCode(501), false);

	if (ContentLength && body.contentLength == -1)
		return (setStatusCode(400), false);
	
	if (!headerExists("Host") || !headerExists("Connection"))
		return (setStatusCode(400), false);
	
	if (header.host.empty() || header.connection.empty())
		return (setStatusCode(400), false);

	int pos = header.contentType.find("multipart/form-data; boundary=") + 30;
	if (headerExists("Content-Type") && pos != 30)
		return (setStatusCode(501), false);
	else if (headerExists("Content-Type") && pos == 30 && header.contentType[pos])
		body.boundaryBegin = "--" + header.contentType.substr(pos), body.boundaryEnd = body.boundaryBegin + "--", isMultipart = true;

	return true;
}

// HEADERS CONTROL CENTER
bool			Request::parseRequestLineAndHeaders() {
	if (!storeHeadersInVector())
		std::cout << "[ERROR]\tstoreHeadersInVector();" << std::endl;
	if (!parseRequestLine())
		std::cout << "[ERROR]\tparseRequestLine();" << std::endl;
	if (!parseHeaders())
		std::cout << "[ERROR]\tparseHeaders();" << std::endl;
	if (!validateRequestHeaders())
		std::cout << "[ERROR]\tvalidateRequestHeaders();" << std::endl;
	if (statusCode == 200)
		pState = HEADERS_FINISHED;
	else
		pState = PARSING_FINISHED;
	return true;
}
