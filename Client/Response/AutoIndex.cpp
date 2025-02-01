#include "Response.hpp"
#include "Error.hpp"

void	Response::autoIndex()
{
	dirList = opendir(input.path.c_str()); // CHECK LEAK
	if (dirList == NULL)
	{
		std::cerr << "[WEBSERV][ERROR]\t>";
		perror("opendir");
		throw(ErrorPage(500));
	}
	buffer.append("<html>\n"
				"<head>\n"
				"<title>Index of " + input.uri + "</title>\n"
				"</head>\n"
				"<body>\n"
				"<h1>Index of " + input.uri + "</h1>\n"
				"<hr>\n"
				"<pre>\n");
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
		nextState = FINISHED;
	}
	buffer = buildChunk(buffer.c_str(), buffer.size());
	state = SENDDATA;
}
