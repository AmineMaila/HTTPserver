/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nazouz <nazouz@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 18:26:22 by nazouz            #+#    #+#             */
/*   Updated: 2025/03/02 23:41:09 by nazouz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

bool			Request::headerExists(const std::string& key) {
	std::map<std::string, std::string>::iterator it;
	it = _RequestData.Headers.find(key);
	if (it == _RequestData.Headers.end())
		return false;
	return true;
}

bool						Request::isCriticalHeader(const std::string& key) {
	if (key == "content-length" || key == "host" || key == "authorization" || key == "content-type" || key == "connection")
		return true;
	return false;
}

void						Request::decodeURI() {
	std::string			encodedURI = _RequestData.URI;
	std::string			decodedURI;

	for (size_t i = 0; i < encodedURI.size(); i++) {
		if (encodedURI[i] == '%' && i + 2 < encodedURI.size() && isHexa(encodedURI.substr(i + 1, 2))) {
			decodedURI += htoi(encodedURI.substr(i + 1, 2));
			i += 2;
		} else if (encodedURI[i] == '+') {
			decodedURI += ' ';
		} else {
			decodedURI += encodedURI[i];
		}
	}
	_RequestData.URI = decodedURI;
}

void						Request::isValidMethod() {
	if (_RequestData.Method != "GET" && _RequestData.Method != "POST" && _RequestData.Method != "DELETE")
		throw (Code(501));
	
	if (std::find(_RequestData._Config->methods.begin(), _RequestData._Config->methods.end(), _RequestData.Method) == _RequestData._Config->methods.end())
		throw (Code(405));
}

void						Request::isValidURI() {
	if (_RequestData.URI.empty() || _RequestData.URI[0] != '/')
		throw (Code(400));
	
	if (_RequestData.URI.size() > 2048)
		throw (Code(414));
	
	static const char allowedURIChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ._-~:/?#[]@!$&'()*+,;=%";
	if (_RequestData.URI.find_first_not_of(allowedURIChars) != std::string::npos)
		throw (Code(400));
	
	decodeURI();
}

void						Request::isValidHTTPVersion() {
	if (_RequestData.HTTPversion.size() != 8 || _RequestData.HTTPversion.find("HTTP/") != 0)
		throw (Code(400));
	
	if (_RequestData.HTTPversion.find("1.1", 5) != 5)
		throw (Code(505));
}


void						Request::parseRequestLine() {
	size_t					CRLFpos = buffer.find(CRLF);

	_RequestRaws.rawRequestLine = buffer.substr(0, CRLFpos);
	buffer.erase(0, CRLFpos + 2);
	bufferSize -= CRLFpos + 2;
	
	if (std::count(_RequestRaws.rawRequestLine.begin(), _RequestRaws.rawRequestLine.end(), ' ') != 2)
		throw (Code(400));
	
	std::stringstream		RLss(_RequestRaws.rawRequestLine);

	std::getline(RLss, _RequestData.Method, ' ');
	isValidMethod();
	
	std::getline(RLss, _RequestData.URI, ' ');
	isValidURI();
	
	std::getline(RLss, _RequestData.HTTPversion, ' ');
	isValidHTTPVersion();
	
}

void						Request::parseRequestHeaders() {
	size_t					CRLFpos = buffer.find(DOUBLE_CRLF);
	std::stringstream		Hss(buffer.substr(0, CRLFpos + 4));

	buffer.erase(0, CRLFpos + 4);
	bufferSize -= CRLFpos + 4;

	std::string			fieldline;
	static const char	allowedValChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_ :;.,\\/\"\'?!(){}[]@<>=-+*#$&`|~^%";
	static const char	allowedKeyChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
	
	while (std::getline(Hss, fieldline) && fieldline != "\r") {
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
			_RequestData.Headers[fieldName] += ", " + fieldValue;
		_RequestData.Headers[fieldName] = fieldValue;
	}
}

void						Request::validateRequestHeaders() {
	_RequestData.host = headerExists("host") ? _RequestData.Headers["host"] : ""; // searching twice for no reason + creating empty string for no reason
	if (_RequestData.host.empty())
		throw (Code(400));
	
	if (headerExists("connection"))
	{
		if (_RequestData.Headers["connection"] == "close")
			_RequestData.keepAlive = false;
	}

	if (_RequestData.Method == "POST") {
		bool		ContentLength = headerExists("content-length");
		bool		TransferEncoding = headerExists("transfer-encoding");
		bool		ContentType = headerExists("content-type");


		if (!TransferEncoding && !ContentLength)
			throw (Code(411));
		if (TransferEncoding && ContentLength)
			_RequestData.Headers.erase("content-length"), ContentLength = false;
		
		if (TransferEncoding) {
			_RequestData.transferEncoding = stringtolower(_RequestData.Headers["transfer-encoding"]);
			if (_RequestData.transferEncoding != "chunked")
				throw (Code(501));
			isEncoded = true;
		}
		if (ContentLength) {
			if (!stringIsDigit(_RequestData.Headers["content-length"]))
				throw (Code(400));
			char	*stringstop;
			_RequestData.contentLength = std::strtoul(_RequestData.Headers["content-length"].c_str(), &stringstop, 10);
			if (ERANGE == errno)
				throw (Code(413));
			if (EINVAL == errno)
				throw (Code(400));
			if (_RequestData.contentLength > _RequestData._Config->client_max_body_size)
				throw (Code(413));
		}

		if (ContentType) {
			_RequestData.contentType = _RequestData.Headers["content-type"];
			
			size_t		semiColonPos = _RequestData.contentType.find(';');
			if (semiColonPos == std::string::npos)
				return ;
			
			std::string mediaType = _RequestData.contentType.substr(0, semiColonPos);
			if (mediaType.empty())
				throw (Code(400));
			isMultipart = (mediaType == "multipart/form-data");
			if (!isMultipart)
				return ;

			size_t	boundaryPos = _RequestData.contentType.find("boundary=", semiColonPos);
			if (boundaryPos == std::string::npos)
				throw (Code(400));
			
			_RequestRaws.boundaryBegin = "--" + _RequestData.contentType.substr(boundaryPos + 9);
			if (_RequestRaws.boundaryBegin.empty() || _RequestRaws.boundaryBegin.size() > 72) // RFC 2046, Section 5.1.1
				throw (Code(400));
			_RequestRaws.boundaryEnd += _RequestRaws.boundaryBegin + "--";
		}
		else
			_RequestData.contentType = "application/octet-stream";
	}
}

// HEADERS CONTROL CENTER
void						Request::parseRequestLineAndHeaders() {
	parseRequestLine();
	parseRequestHeaders();
	validateRequestHeaders();
	headersFinished = true;
}
