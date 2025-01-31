/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmaila <mmaila@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 17:46:13 by nazouz            #+#    #+#             */
/*   Updated: 2025/01/30 19:11:41 by mmaila           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

# include <iostream>
# include <iomanip>
# include <sstream>
# include <string.h>
# include <map>
# include <vector>

# include <fcntl.h>
# include <sys/socket.h>
# include <unistd.h>
# include "../../Utils/Helpers.hpp"

# define BUFFER_SIZE 16000

enum e_parsingState {
	PARSING_INIT,		// 0
	HEADERS_RECEIVED,	// 1
	HEADERS_FINISHED,	// 2
	BODY_RECEIVED,		// 3
	BODY_FINISHED,		// 4
	PARSING_FINISHED	// 5
};

typedef struct								s_requestline {
	std::string								rawRequestLine;
	std::string								method;
	std::string								uri;
	std::string								httpversion;
	// std::string								query;
}											t_requestline;

typedef struct								s_header {
	std::vector<std::string>				rawHeaders;
	std::map<std::string, std::string>		headersMap;
	std::string								host;
	std::string								contentType;
	std::string								connection;
	std::string								transferEncoding;
	// std::string								contentLength;
}											t_header;

typedef struct								s_body {
	std::string								rawBody;
	std::string								boundaryBegin;
	std::string								boundaryEnd;
	int										bodySize;
	int										contentLength;
}											t_body;

class Request {
	private:
		// int								rawBodyFD;
		// int								bufferFD;
		// int								debugFD;
		
		/*			 PARSING BUFFRER			*/
		std::string						buffer;
		int								bufferSize;

		std::vector<int>				files;

		/*			PARSING STRUCTURES			*/
		t_body							body;
		t_header						header;
		t_requestline					requestLine;

		/*			   PARSING FLAGS			*/
		bool							isEncoded;
		bool							isMultipart;

		/*				STATE FLAGS				*/
		e_parsingState					pState;
		int								statusCode;
	
	public:
		Request();
		// Request(const std::string& rawRequest);
		~Request();
		Request(const Request& rhs);
		Request&	operator=(const Request& rhs);

		int							receiveRequest(int socket);
		bool						printParsedRequest();

		bool						bufferContainHeaders();
		bool						bufferContainChunk();
		std::string					extractHeadersFromBuffer();
		bool						headerExists(const std::string& key);
		void						feedRequest(char *recvBuffer, int bufferSize);

		bool						parseControlCenter();
		void						setRequestState();
		bool						parseRequestLineAndHeaders();
		bool						storeHeadersInVector();
		bool						parseRequestLine();
		bool						parseHeaders();
		bool						parseRequestBody();
		bool						decodeChunkedBody();
		bool						processRequestRawBody();
		bool						processMultipartFormData();
		bool						processMultipartHeaders();
		bool						processMultipartData();
		bool						processBinaryBody();
		bool						parseLengthBody();
		bool						validateRequestHeaders();

		void						putRequestBodyInFile();

		std::string&				getBuffer() { return buffer; };
		void						setBuffer(const std::string& newValue) { this->buffer = newValue; };

		int&						getStatusCode();
		std::vector<std::string>&	getRawRequest();
		t_body&						getBodySt();
		t_header&					getHeaderSt();
		t_requestline&				getRequestLineSt();
		e_parsingState				getParsingState() { return pState; } ;

		void						setStatusCode(int code);
		bool						decodeURI();
		bool						isValidFieldLine(const std::string& fieldline);
		bool						isValidMethod(const std::string& method);
		bool						isValidURI(const std::string& uri);
		bool						isValidHTTPVersion(const std::string& httpversion);
		
		
};

bool			stringIsDigit(const std::string& str);
std::string		stringtrim(const std::string& str, const std::string& set);
std::string		stringtolower(std::string& str);
bool			isHexa(const std::string& num);
int				hexToInt(const std::string& num);

#endif