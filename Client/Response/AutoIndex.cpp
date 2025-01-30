#include "Response.hpp"

void	Response::autoIndex()
{
	buffer.append("<html>\n"
				"<head>\n"
				"<title>Index of " + input.uri + "</title>\n"
				"</head>\n"
				"<body>\n"
				"<h1>Index of " + input.uri + "</h1>\n"
				"<hr>\n"
				"<pre>\n");
	nextState = LISTDIR;
	directoryListing();
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
		nextState = FINISHED;
	}
	buildChunk();
}
