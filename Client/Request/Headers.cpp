/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nazouz <nazouz@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 18:26:22 by nazouz            #+#    #+#             */
/*   Updated: 2025/03/07 22:50:52 by nazouz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

bool	Request::headerExists(const std::string& key)
{
	std::map<std::string, std::string>::iterator it;
	it = requestData.Headers.find(key);
	if (it == requestData.Headers.end())
		return false;
	return true;
}

bool	Request::isCriticalHeader(const std::string& key)
{
	if (key == "content-length" || key == "host" || key == "authorization" || key == "content-type" || key == "connection")
		return true;
	return false;
}

void	Request::decodeURI()
{
	std::string			encodedURI = requestData.URI;
	std::string			decodedURI;

	for (size_t i = 0; i < encodedURI.size(); i++)
	{
		if (encodedURI[i] == '%' && i + 2 < encodedURI.size() && isHexa(encodedURI.substr(i + 1, 2)))
		{
			decodedURI += htoi(encodedURI.substr(i + 1, 2));
			i += 2;
		}
		else if (encodedURI[i] == '+')
			decodedURI += ' ';
		else
			decodedURI += encodedURI[i];
	}
	requestData.URI = decodedURI;
}

void	Request::validateMethod()
{
	if (requestData.Method != "GET" && requestData.Method != "POST" && requestData.Method != "DELETE")
		throw (Code(501));
}

void	Request::validateURI()
{
	if (requestData.URI.empty() || requestData.URI[0] != '/')
		throw (Code(400));
	
	if (requestData.URI.size() > 2048)
		throw (Code(414));
	
	static const char allowedURIChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ._-~:/?#[]@!$&'()*+,;=%";
	if (requestData.URI.find_first_not_of(allowedURIChars) != std::string::npos)
		throw (Code(400));
	
	decodeURI();
	requestData.URI = normalizeURI(requestData.URI);
}

void	Request::validateHTTPVersion()
{
	if (requestData.HTTPversion.size() != 8 || requestData.HTTPversion.find("HTTP/") != 0)
		throw (Code(400));
	
	if (requestData.HTTPversion.find("1.1", 5) != 5)
		throw (Code(505));
}


void	Request::parseRequestLine()
{
	size_t	CRLFpos = buffer.find(CRLF);

	requestRaws.rawRequestLine = buffer.substr(0, CRLFpos);
	buffer.erase(0, CRLFpos + 2);
	
	if (std::count(requestRaws.rawRequestLine.begin(), requestRaws.rawRequestLine.end(), ' ') != 2)
		throw (Code(400));
	
	std::stringstream		RLss(requestRaws.rawRequestLine);

	std::getline(RLss, requestData.Method, ' ');
	validateMethod();
	
	std::getline(RLss, requestData.URI, ' ');
	validateURI();
	
	std::getline(RLss, requestData.HTTPversion, ' ');
	validateHTTPVersion();
}

void	Request::parseHeaders()
{
	size_t				CRLFpos = buffer.find(DOUBLE_CRLF);
	std::stringstream	Hss(buffer.substr(0, CRLFpos + 4));

	buffer.erase(0, CRLFpos + 4);

	std::string			fieldline;
	static const char	allowedValChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_ :;.,\\/\"\'?!(){}[]@<>=-+*#$&`|~^%";
	static const char	allowedKeyChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
	
	while (std::getline(Hss, fieldline) && fieldline != "\r")
	{
		if (fieldline.size() > 4096) // 4KB
			throw(Code(431));
		size_t		colonPos = fieldline.find(':');

		if (colonPos == std::string::npos)
			throw (Code(400));
		
		std::string		fieldName = stringtolower(fieldline.substr(0, colonPos));
		if (fieldName.empty() || fieldName.find_first_not_of(allowedKeyChars) != std::string::npos)
			throw (Code(400));
		
		bool			headerExist = headerExists(fieldName);
		if (isCriticalHeader(fieldName) && headerExist)
			throw (Code(400));
		
		std::string		fieldValue = stringtrim(fieldline.substr(colonPos + 1), " \r\n\t\v");
		if (fieldValue.empty() || fieldName.find_first_not_of(allowedValChars) != std::string::npos)
			throw (Code(400));
		if (headerExist)
			requestData.Headers[fieldName] += ", " + fieldValue;
		requestData.Headers[fieldName] = fieldValue;
	}
}

void						Request::validateHeaders() {

	std::map<std::string, std::string>::iterator field;

	field = requestData.Headers.find("host");
	if (field == requestData.Headers.end())
		throw(Code(400));
	
	requestData.host = field->second;

	size_t		colonPos = requestData.host.find(':');
	if (colonPos != std::string::npos)
		requestData.serverPort = requestData.host.substr(colonPos + 1);
	requestData.serverHost = requestData.host.substr(0, colonPos);
	if (requestData.serverHost.empty() || requestData.serverPort.empty())
		throw (Code(400));
	
	field = requestData.Headers.find("connection");
	if (field != requestData.Headers.end())
		requestData.keepAlive = (field->second == "keep-alive") ? true : false;
	
	field = requestData.Headers.find("transfer-encoding");

	if (field != requestData.Headers.end())
	{
		requestData.transferEncoding = stringtolower(field->second);
		if (requestData.transferEncoding != "chunked")
			throw (Code(501));
		requestData.isEncoded = true;
	}
	
	field = requestData.Headers.find("content-length");

	if (field != requestData.Headers.end())
	{
		if (requestData.Method == "POST" && requestData.isEncoded)
			throw(Code(411));
		if (!stringisdigit(field->second))
			throw (Code(400));
		char	*stringstop;
		requestData.contentLength = std::strtoul(field->second.c_str(), &stringstop, 10);
		if (ERANGE == errno || EINVAL == errno)
			throw (Code(400));
	}
	else if (requestData.Method == "POST" && !requestData.isEncoded)
		throw (Code(411));

	field = requestData.Headers.find("content-type");

	if (field != requestData.Headers.end())
	{
		requestData.contentType = field->second;
		
		size_t		semiColonPos = requestData.contentType.find(';');
		if (semiColonPos == std::string::npos)
			return ;
		
		std::string mediaType = requestData.contentType.substr(0, semiColonPos);
		if (mediaType.empty())
			throw (Code(400));
		requestData.isMultipart = (mediaType == "multipart/form-data");
		if (!requestData.isMultipart)
			return ;
	
		size_t	boundaryPos = requestData.contentType.find("boundary=", semiColonPos);
		if (boundaryPos == std::string::npos)
			throw (Code(400));
		
		requestRaws.boundaryBegin = "--" + requestData.contentType.substr(boundaryPos + 9);
		if (requestRaws.boundaryBegin.empty() || requestRaws.boundaryBegin.size() > 72) // RFC 2046, Section 5.1.1
			throw (Code(400));
		requestRaws.boundaryEnd += requestRaws.boundaryBegin + "--";
	}
	else
		requestData.contentType = "application/octet-stream";
}
