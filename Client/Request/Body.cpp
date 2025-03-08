/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Body.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmaila <mmaila@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/21 10:30:24 by nazouz            #+#    #+#             */
/*   Updated: 2025/03/08 19:32:43 by mmaila           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

bool	Request::bufferContainChunk()
{
	size_t		cpos = 0;
	size_t		CRLFpos = 0;
	std::string	chunkSizeStr;

	CRLFpos = buffer.find(CRLF);
	if (CRLFpos == std::string::npos)
		return false;
	
	chunkSizeStr = buffer.substr(cpos, CRLFpos - cpos);
	if (!isHexa(chunkSizeStr))
		return false;
	
	if (buffer.find(CRLF, CRLFpos + 2) == std::string::npos)
		return false;
	return true;
}

void	Request::parseLengthBody()
{
	if (buffer.empty())
		return ;

	if (requestRaws.totalBodySize + buffer.size() < requestRaws.totalBodySize)
		throw Code(413);
	if (requestRaws.totalBodySize + buffer.size() > requestData.config->client_max_body_size)
		throw Code(413);
	if (requestRaws.totalBodySize + buffer.size() > requestData.contentLength)
		throw Code(400);

	requestRaws.rawBody.append(buffer);
	requestRaws.rawBodySize += buffer.size();
	requestRaws.totalBodySize += buffer.size();
	buffer.clear();
	
	if (requestRaws.totalBodySize == requestData.contentLength)
		bodyStored = true;
	
}

void	Request::unchunk()
{
	while (bufferContainChunk())
	{
		std::string	chunkSizeStr;
		size_t		currPos = 0, CRLFpos = 0;
		
		CRLFpos = buffer.find(CRLF);
		
		chunkSizeStr = buffer.substr(currPos, CRLFpos - currPos);
		if (!isHexa(chunkSizeStr))
			throw Code(400);
		
		size_t chunkSize = htoi(chunkSizeStr);
		if (!chunkSize)
		{
			if (buffer.substr(CRLFpos, 4) == DOUBLE_CRLF)
			{
				buffer.clear();
				bodyStored = true;
			}
			else
				throw (Code(400));
		}
		else
		{
			currPos = CRLFpos + 2;
			if (buffer.size() > currPos + chunkSize + 2)
			{
				if (requestRaws.totalBodySize + chunkSize < requestRaws.totalBodySize)
					throw Code(413);
				if (requestRaws.totalBodySize + chunkSize > requestData.config->client_max_body_size)
					throw Code(413);
				
				requestRaws.rawBody += buffer.substr(currPos, chunkSize);
				requestRaws.rawBodySize += chunkSize;
				requestRaws.totalBodySize += chunkSize;
				buffer.erase(0, currPos + chunkSize + 2);
			}
		}
	}
}

void	Request::processMultipartHeaders()
{
	size_t		currPos, filenamePos, filenamePos_;
	std::string	contentDisposition;
	std::string	filename;

	currPos = requestRaws.boundaryBegin.size() + 2;
	contentDisposition = requestRaws.rawBody.substr(currPos, requestRaws.rawBody.find(CRLF, currPos));
	filenamePos = contentDisposition.find("filename=\"") + 10;
	if (filenamePos == std::string::npos)
		throw Code(400);
	filenamePos_ = contentDisposition.find("\"", filenamePos);
	if (filenamePos_ == std::string::npos)
		throw Code(400);
	
	filename = contentDisposition.substr(filenamePos, filenamePos_ - filenamePos);
	if (filename.empty())
		throw Code(400);
	
	if (access((requestData.config->upload_store + filename).c_str(), F_OK) == 0)
		throw Code(409);
	
	fileUploader.open(std::string(requestData.config->upload_store + filename).c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
	if (!fileUploader.is_open())
		throw Code(500);
	
	currPos = requestRaws.rawBody.find(DOUBLE_CRLF, currPos);
	if (currPos == std::string::npos)
	{
		fileUploader.close();
		throw Code(400);
	}
	currPos += 4;
	requestRaws.rawBody.erase(0, currPos);
	requestRaws.rawBodySize -= currPos;
}

void	Request::processMultipartData()
{
	size_t	bboundary = 0, eboundary = 0;
	
	bboundary = requestRaws.rawBody.find(CRLF + requestRaws.boundaryBegin + CRLF);
	eboundary = requestRaws.rawBody.find(CRLF + requestRaws.boundaryEnd + CRLF);
	
	if (!fileUploader.is_open())
		throw Code(500);
	
	int bytesToWrite = 0;
	if (bboundary == std::string::npos && eboundary == std::string::npos)
	{
		bytesToWrite = requestRaws.rawBodySize;

		fileUploader.write(requestRaws.rawBody.c_str(), requestRaws.rawBody.size());
		if (fileUploader.fail())
		{
			fileUploader.close();
			throw Code(500);
		}
		
		requestRaws.rawBody.clear();
		requestRaws.rawBodySize = 0;
		return ;
	}
	else if (bboundary != std::string::npos)
		bytesToWrite = bboundary;
	else
		bytesToWrite = eboundary;
	
	fileUploader.write(requestRaws.rawBody.substr(0, bytesToWrite).c_str(), bytesToWrite);
	if (fileUploader.fail())
	{
		fileUploader.close();
		throw Code(500);
	}
	
	fileUploader.close();
	requestRaws.rawBody.erase(0, bytesToWrite + 2);
	requestRaws.rawBodySize -= bytesToWrite + 2;
}

void			Request::processMultipart()
{
	while (!requestRaws.rawBody.empty())
	{
		if (requestRaws.rawBody.find(requestRaws.boundaryBegin + CRLF) == 0 && requestRaws.rawBody.find(DOUBLE_CRLF) != std::string::npos)
			processMultipartHeaders();
		if (requestRaws.rawBody.find(requestRaws.boundaryBegin + CRLF) != 0 && fileUploader.is_open())
			processMultipartData();
		if (requestRaws.rawBody == requestRaws.boundaryEnd + CRLF)
			requestRaws.rawBody.clear(), requestRaws.rawBodySize = 0, fileUploader.close(), requestData.statusCode = 201;
	}
}

void			Request::processBinaryBody()
{
	if (requestRaws.rawBody.empty())
		return ;

	if (!fileUploader.is_open())
	{
		std::string	fileUploaderRandname = "file_" + generateRandomString();
		
		size_t	semiColonPos = requestData.contentType.find(';');
		
		std::string	contentType;
		if (semiColonPos != std::string::npos)
			contentType = requestData.contentType.substr(0, semiColonPos);
		else
			contentType = requestData.contentType;
		
		if (requestRaws.mimeTypes.find(requestData.contentType) == requestRaws.mimeTypes.end())
			throw Code(415);
		
		if (access((requestData.config->upload_store + fileUploaderRandname 
			+ requestRaws.mimeTypes[contentType]).c_str(), F_OK) == 0)
			throw Code(409);
		
		fileUploader.open(std::string(requestData.config->upload_store + fileUploaderRandname 
			+ requestRaws.mimeTypes[contentType]).c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
	}
	
	if (!fileUploader.is_open())
		throw Code(500);
	
	fileUploader.write(requestRaws.rawBody.c_str(), requestRaws.rawBodySize);
	if (fileUploader.fail())
	{
		fileUploader.close();
		throw Code(500);
	}
	
	requestRaws.rawBody.clear();
	requestRaws.rawBodySize = 0;

	if (bodyStored)
	{
		fileUploader.close();
		requestData.statusCode = 201;
	}
}

void			Request::processREGBody()
{
	if (!requestRaws.rawBodySize)
		return ;

	if (requestData.isMultipart)
		processMultipart();
	else
		processBinaryBody();
}

void	Request::openTmpFile()
{
	std::string		randomName = "/tmp/webserv_" + generateRandomString();
	
	if (access(randomName.c_str(), F_OK) == 0)
		throw Code(409);
	
	requestData.tmpFileName = randomName;
	fileUploader.open(requestData.tmpFileName.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
	if (!fileUploader.is_open())
		throw Code(500);
}

void	Request::processCGIBody()
{
	fileUploader.write(requestRaws.rawBody.c_str(), requestRaws.rawBodySize);
	if (fileUploader.fail()) {
		fileUploader.close();
		throw Code(500);
	}

	requestRaws.rawBody.clear();
	requestRaws.rawBodySize = 0;
	if (bodyStored)
		fileUploader.close();
}

// BODY CONTROL CENTER
void	Request::processBody()
{
	if (requestData.isEncoded)
		unchunk();
	else
		parseLengthBody();

	if (requestData.isCGI)
		processCGIBody();
	else
		processREGBody();
}
