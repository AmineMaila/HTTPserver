# include "Response.hpp"

void	Response::parseRangeHeader( void ) // example => Range: bytes=0-499,1000-1499
{
	std::string	value = reqCtx->Headers["range"];
	std::string prefix = "bytes=";
	size_t pos = value.find(prefix);
	if (pos == std::string::npos)
		throw(Code(416));
	pos += prefix.length();


	std::istringstream	rangess(value.substr(pos));
	std::string			rangeStr;

	rangeData.boundary = generateRandomString();

	while (std::getline(rangess, rangeStr, ','))
	{
		unsigned long start;
		unsigned long end;
		std::string startStr;
		std::string endStr;
		size_t	delim;
		char	*stop;

		stringtrim(rangeStr, " \t");
		delim = rangeStr.find("-");
		if (delim == std::string::npos)
			throw(Code(416));

		startStr = rangeStr.substr(0, delim);
		endStr = rangeStr.substr(delim + 1);
		if (startStr.empty() && endStr.empty())
			throw(Code(416));

		if (!startStr.empty())
		{
			start = std::strtoul(startStr.c_str(), &stop, 10); // 10: base decimal
			if (errno == ERANGE || errno == EINVAL)
				throw(Code(416));
			if (endStr.empty())
				end = contentLength - 1;
		}
		if (!endStr.empty())
		{
			end = std::strtoul(endStr.c_str(), &stop, 10); // 10: base decimal
			if (errno == ERANGE || errno == EINVAL)
				throw(Code(416));
			if (startStr.empty())
			{
				start = contentLength - end;
				end = contentLength - 1;
			}
		}

		if (start > end || end >= contentLength)
			throw(Code(416));

		Range unit;

		unit.range = std::make_pair(start, end);
		unit.rangeLength = end - start + 1;
		unit.header.append("\r\n--" + rangeData.boundary);
		unit.header.append("\r\nContent-Range: bytes " + startStr + "-" + endStr + "/" + _toString(contentLength));
		unit.header.append("\r\nContent-Type: " + contentType);
		unit.header.append("\r\n\r\n");

		rangeData.ranges.push_back(unit);
	}
	rangeData.ranges[0].header.erase(0, 2); // erases the first "\r\n"
	rangeData.current = rangeData.ranges.begin();
}

int	Response::rangeContentLength( void )
{
	int length = 0;
	std::vector<Range>::iterator it = rangeData.ranges.begin();

	for (; it != rangeData.ranges.end(); it++)
	{
		length += it->header.length();
		length += it->rangeLength;
	}
	length += 8 + rangeData.boundary.length(); // 8 is  the length of the constant "\r\n--" "--\r\n"
	return (length);
}

void	Response::handleRange()
{
	reqCtx->StatusCode = 206;
	parseRangeHeader();
	std::cout << "content-length: " << contentLength << std::endl;
	if (rangeData.ranges.size() == 1)
	{
		headers.append("\r\nContent-Range: bytes " + _toString(rangeData.current->range.first) + "-" + _toString(rangeData.current->range.second) + "/" + _toString(contentLength));
	}
	else
	{
		contentType = "multipart/byteranges; boundary=" + rangeData.boundary;
		contentLength = rangeContentLength();
	}
	reader = &Response::range;
}

void	Response::nextRange()
{
	if (rangeData.current == rangeData.ranges.end())
	{
		if (rangeData.ranges.size() > 1)
		{
			buffer = "\r\n--" + rangeData.boundary + "--\r\n";
			nextState = DONE;
			if ((this->*sender)() == true)
				state = nextState;
			else
				state = WRITE;
		}
		else if (rangeData.ranges.size() == 1)//////// FIIIIX
		{
			state = DONE;
		}
	}
	else
	{
		if (rangeData.ranges.size() > 1)
			buffer.append(rangeData.current->header);
		bodyFile.seekg(rangeData.current->range.first, std::ios::beg);
		rangeData.rangeState = GET;
	}
}

void		Response::range()
{
	switch (rangeData.rangeState)
	{
		case NEXT:
			nextRange();
			break;
		case GET:
			readRange();
			break;
	}
}