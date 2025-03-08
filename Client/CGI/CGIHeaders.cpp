#include "CGIHandler.hpp"

void	CGIHandler::validateHeaders()
{
	std::map<std::string, std::string>::iterator field;

	// Content-Type
	field = headersMap.find("content-type");
	if (field == headersMap.end() && !buffer.empty())
		throw(Code(500));

	// ContentLength
	field = headersMap.find("content-length");
	if (field == headersMap.end() && !buffer.empty())
	{
		headers.append("\r\nTransfer-Encoding: chunked");
		chunked = true;
		buffer = buildChunk(buffer.c_str(), buffer.size());
	}

	// Status
	std::pair<std::string, std::string> statusMsg;
	field = headersMap.find("status");
	if (headersMap.find("status") != headersMap.end())
	{
		size_t splitPos = field->second.find_first_of(' ');
		if (splitPos != std::string::npos)
			statusMsg.second = field->second.substr(splitPos + 1);
		statusMsg.first = field->second.substr(0, splitPos);

		char *stop;
		reqCtx->statusCode = strtoul(statusMsg.first.c_str(), &stop, 10);
		if (errno == ERANGE || errno == EINVAL)
			throw(Code(500));
		headers.insert(0, "HTTP/1.1 " + statusMsg.first + " " + statusMsg.second);

		headersMap.erase(field);
	}
	else
		headers.insert(0, "HTTP/1.1 " + _toString(reqCtx->statusCode) + " " + getCodeDescription(reqCtx->statusCode));

	// Location
	field = headersMap.find("location");
	if (headersMap.find("location") != headersMap.end())
	{
		if (field->second.at(0) == '/')
			throw(CGIRedirect(field->second));
		if (statusMsg.first.empty())
			throw(Code(302, field->second));
	}
}

void	CGIHandler::parseHeaders()
{
	size_t CRLFpos = buffer.find("\r\n\r\n");
	if (CRLFpos == std::string::npos)
		throw(Code(500));
	size_t	start = 0;
	size_t	end = 0;

	while (start < CRLFpos)
	{
		end = buffer.find("\r\n", start);
		if (end == std::string::npos || end > CRLFpos)
			break;
	
		std::string field = buffer.substr(start, end - start);

		size_t pos = field.find(':');
		if (pos == std::string::npos)
			throw(Code(500));

		std::string key = stringtolower(field.substr(0, pos));
		std::string value = stringtrim(field.substr(pos + 1), " \t\v\f");
		if (value.empty())
		{
			start = end + 2;
			continue ; // RFC 3875 section 6.3
		}

		for (size_t i = 0; i < key.size(); i++)
        {
            if (!(std::isalnum(key[i]) || key[i] == '-'))
                throw Code(500);
        }
		headersMap[key] = value;
	
		start = end + 2;
	}

	buffer.erase(0, CRLFpos + 4);
}

void	CGIHandler::addHeaders()
{
	std::map<std::string, std::string>::iterator field;

	for (field = headersMap.begin(); field != headersMap.end(); field++)
		headers.append("\r\n" + capitalize(field->first) + ": " + field->second);
}

void	CGIHandler::generateHeaders()
{
	if (buffer.size() < 8192 && nextState != DONE)
	{
		state = READ;
		nextState = HEADERS;
		return ;
	}
	headers.reserve(1024);

	headers.append("\r\nServer: webserv/1.0");
	headers.append("\r\nDate: " + getDate());

	if (reqCtx->keepAlive)
		headers.append("\r\nConnection: keep-alive");
	else
		headers.append("\r\nConnection: close");

	parseHeaders();
	validateHeaders();
	addHeaders();
	headers.append("\r\n\r\n");
	headersParsed = true;
	nextState = READ;
	if ((this->*sender)() == true && buffer.empty())
		state = nextState;
	else
		state = WRITE;
}