#include "Response.hpp"
#include "Error.hpp"

void	Response::initDirList()
{
	dirList = opendir(reqCtx->fullPath.c_str());
	if (!dirList)
	{
		throw(500);
	}
	buffer = "<html>\n"
			"<head>\n"
			"<title>Index of " + reqCtx->URI + "</title>\n"
			"</head>\n"
			"<body>\n"
			"<h1>Index of " + reqCtx->URI + "</h1>\n"
			"<hr>\n"
			"<pre>\n";
}

void	Response::directoryListing()
{
	struct dirent	*entry;
	int 			i = 0;

	while (i < 100 && (entry = readdir(dirList)) != NULL)
	{
		std::string name = entry->d_name;
		if (name == ".")
			continue ;
		if (entry->d_type == DT_DIR)
			name.append("/");
		buffer.append("<a href=\"" + name + "\">" + name + "</a>\n");
		i++;
	}
	if (entry == NULL)
	{
		buffer.append("</pre>\n"
					"<hr>\n"
					"</body>\n"
					"</html>");
		closedir(dirList);
			dirList = NULL;
		nextState = DONE;
	}
	buffer = buildChunk(buffer.c_str(), buffer.size());
	state = WRITE;
}

void	Response::readRange()
{
	char buf[SEND_BUFFER_SIZE] = {0};

	size_t readLength = std::min
	(
		static_cast<size_t>(SEND_BUFFER_SIZE),
		reqCtx->rangeData.current->rangeLength
	);
	std::cout << "READ LENGTH OF RANGE :" << readLength << std::endl;
	int bytesRead = bodyFile.read(buf, readLength).gcount();
	if (bytesRead == -1)
	{
		throw(FatalError(strerror(errno)));
	}
	else if (bytesRead > 0)
	{
		buffer.append(std::string(buf, bytesRead));
		std::cout << YELLOW << "======[READ DATA OF SIZE " << bytesRead << "]======" << RESET << std::endl;
		reqCtx->rangeData.current->rangeLength -= bytesRead;
		state = WRITE;
		if (reqCtx->rangeData.current->rangeLength == 0)
		{
			reqCtx->rangeData.rangeState = NEXT;
			reqCtx->rangeData.current++;
		}
	}
}

void	Response::readBody()
{
	char buf[SEND_BUFFER_SIZE] = {0};
	int bytesRead = bodyFile.read(buf, SEND_BUFFER_SIZE).gcount();
	if (bytesRead == -1)
	{
		throw (FatalError(strerror(errno)));
	}
	else if (bytesRead > 0)
	{
		if (bodyFile.peek() == EOF)
			nextState = DONE;
		std::cout << YELLOW << "======[READ DATA OF SIZE " << bytesRead << "]======" << RESET << std::endl;
		buffer.append(std::string(buf, bytesRead));
		state = WRITE;
	}
}