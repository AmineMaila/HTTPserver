/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _ControlCenter.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmaila <mmaila@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/21 10:39:00 by nazouz            #+#    #+#             */
/*   Updated: 2025/02/07 15:59:16 by mmaila           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

void			Request::setStatusCode(int code) {
	statusCode = code;
	if (code != 200)
		pState = PARSING_FINISHED;
}

void			Request::setRequestState() {
	// if (statusCode)
	// if we haven't received completed headers
	if (pState == PARSING_INIT && bufferContainHeaders())
    {
		pState = HEADERS_RECEIVED;
    }
	else if (pState == HEADERS_FINISHED && bufferSize != 0)
    {
        // std::cout << buffer << std::endl;
		pState = BODY_RECEIVED;
    }
	else if (pState == BODY_FINISHED)
		pState = PARSING_FINISHED;
	
	// if (pState == HEADERS_FINISHED && requestLine.method == "GET")
	// 	pState = PARSING_FINISHED;
	if (pState == HEADERS_FINISHED && _RequestData.Method == "GET")
		pState = PARSING_FINISHED;
}

// PARSING CONTROL CENTER
bool			Request::parseControlCenter() {
	setRequestState();
	switch (pState) {
		case HEADERS_RECEIVED:
			parseRequestLineAndHeaders();
			break;
		case BODY_RECEIVED:
			parseRequestBody();
			break;
		case PARSING_FINISHED:
			break;
		default:
			break;
	}
	setRequestState();
	return true;
}

// FEED REQUEST
int	Request::receiveRequest(int socket) {
	char	buf[REQUEST_BUFFER_SIZE] = {0};

	int	bytesReceived = recv(socket, buf, REQUEST_BUFFER_SIZE, 0);
	if (bytesReceived > 0) {
		std::cout << "----------REQUEST_OF_CLIENT " << socket << "----------" << std::endl;
		std::cout << buf;
		std::cout << "---------------------------------------------------------" << std::endl;
		buffer += std::string(buf, bytesReceived);
		bufferSize += bytesReceived;
		parseControlCenter();
		return (pState);
		
	} else if (bytesReceived == 0) { // this is for graceful shutdown (client closes the connection willingly)
		std::cout << "[SERVER]\tClient " << socket
				<< " disconnected..." << std::endl;
		
		return (-1);
	} else {
		std::cerr << "[ERROR]\tReceiving failed..." << std::endl;
		std::cerr << "[ERROR]\t";
		perror("recv");
		return (-1);
	}
}

// void			Request::feedRequest(char *recvBuffer, int recvBufferSize) {
// 	std::string		recvBuff(recvBuffer, recvBufferSize);
// 	buffer.append(recvBuffer, recvBufferSize);
// 	bufferSize += recvBufferSize;
// 	parseControlCenter();
// 	std::cout << pState << std::endl;
// }
