#include "Response.hpp"

void	Response::initDirList()
{
	dirList = opendir(reqCtx->fullPath.c_str());
	if (!dirList)
		throw(Code(500));
	buffer = "<html>\n"
			"<head><title>Index of " + reqCtx->scriptName + "</title></head>\n"
			"<body>\n"
			"<h1>Index of " + reqCtx->scriptName + "</h1>\n"
			"<hr><pre>\n";
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
		buffer.append("<a href=\"" + reqCtx->scriptName + name + "\">" + name + "</a>\n");
		i++;
	}
	if (entry == NULL)
	{
		buffer.append("</pre><hr>\n"
					"</body>\n"
					"</html>");
		closedir(dirList);
		dirList = NULL;
		nextState = DONE;
		buffer = buildChunk(buffer.c_str(), buffer.size());
		buffer.append("0\r\n\r\n");
	}
	else
		buffer = buildChunk(buffer.c_str(), buffer.size());
	if ((this->*sender)() == true)
		state = nextState;
	else
		state = WRITE;
}

void	Response::readRange()
{
	char buf[SEND_BUFFER_SIZE] = {0};

	size_t readLength = std::min
	(
		static_cast<size_t>(SEND_BUFFER_SIZE),
		rangeData.current->rangeLength
	);
	ssize_t bytesRead = bodyFile.read(buf, readLength).gcount();
	if (bytesRead == -1)
	{
		throw(Disconnect("\tClient " + _toString(socket) + " : read: " + strerror(errno)));
	}
	else if (bytesRead > 0)
	{
		buffer.append(buf, bytesRead);
		rangeData.current->rangeLength -= bytesRead;
		if ((this->*sender)() == true)
			state = nextState;
		else
			state = WRITE;
		if (rangeData.current->rangeLength == 0)
		{
			rangeData.rangeState = NEXT;
			rangeData.current++;
		}
	}
}

void	Response::readBody()
{
	char buf[SEND_BUFFER_SIZE] = {0};
	ssize_t bytesRead = bodyFile.read(buf, SEND_BUFFER_SIZE).gcount();
	if (bytesRead == -1)
	{
		throw(Disconnect("\tClient " + _toString(socket) + " : read: " + strerror(errno)));
	}
	else if (bytesRead > 0)
	{
		if (bodyFile.peek() == EOF)
			nextState = DONE;
		buffer.append(buf, bytesRead);
		if ((this->*sender)() == true)
			state = nextState;
		else
			state = WRITE;
	}
	else
		state = DONE;
}