#include "ClientHandler.hpp"

void	produceAbsPath(RequestData& _RequestData) {
	
	if (!_RequestData._Config->redirect.second.empty())
	{
		if (_RequestData._Config->redirect.second.find("http://") != std::string::npos
			|| _RequestData._Config->redirect.second.find("https://") != std::string::npos)
		{
			throw(Code(_RequestData._Config->redirect.first, _RequestData._Config->redirect.second));
		}
		else
		{
			if (_RequestData._Config->redirect.second.at(0) != '/')
				_RequestData._Config->redirect.second.insert(0, "/");
			throw(Code(_RequestData._Config->redirect.first, "http://" + _RequestData.host + _RequestData._Config->redirect.second));
		}
	}

	if (!_RequestData._Config->alias.empty()) {
		if (_RequestData._Config->alias[_RequestData._Config->alias.length() - 1] == '/')
			_RequestData._Config->alias.erase(_RequestData._Config->alias.length() - 1);
		_RequestData.fullPath = _RequestData._Config->alias + _RequestData.URI.substr(_RequestData.matchingLocation.length());
	}
	else {
		if (_RequestData._Config->root[_RequestData._Config->root.length() - 1] == '/')
			_RequestData._Config->root.erase(_RequestData._Config->root.length() - 1);
		_RequestData.fullPath = _RequestData._Config->root + _RequestData.URI;
	}
}

void	setQueryString(RequestData& _RequestData) {
	size_t		pos;
	std::string	query;

	pos = _RequestData.fullPath.find_last_of('#');
	if (pos != std::string::npos)
		_RequestData.fullPath.erase(pos);
	
	pos = _RequestData.fullPath.find_first_of('?');
	if (pos != std::string::npos)
	{
		_RequestData.queryString = _RequestData.fullPath.substr(pos + 1);
		_RequestData.fullPath.erase(pos);
	}
}

void	setRequestedResourceType(RequestData& _RequestData) {
	struct stat	pathStats;
	std::string	pathChecker;
	
	size_t	startPos = 0;
	size_t	endPos = 0;
	while (startPos < _RequestData.fullPath.size()) {
		endPos = _RequestData.fullPath.find('/', startPos + 1); // check whether it was found or not // no need if it didn't find it it will substr till end of str
		pathChecker.append(_RequestData.fullPath.substr(startPos, endPos - startPos));
		if (stat(pathChecker.c_str(), &pathStats) != 0)
		{
			std::cout << pathChecker <<  "|AAAAAAAAAAAAAAAAAAAAAAAA" << std::endl;
			throw(Code(404));
		}
		if (!S_ISDIR(pathStats.st_mode))
			break ;
		startPos = endPos;
	}
	_RequestData.isDir = S_ISDIR(pathStats.st_mode) ? true : false;
	_RequestData.pathInfo = _RequestData.fullPath.substr(pathChecker.size());
	_RequestData.fullPath.erase(pathChecker.size());
}

void						handleDirectoryResource(RequestData& _RequestData) {
	if (access(_RequestData.fullPath.c_str(), X_OK) != 0)
		throw(Code(403));
	
	if (_RequestData.Method == "GET") {
		std::vector<std::string>::iterator	it = _RequestData._Config->index.begin();
		
		for (; it != _RequestData._Config->index.end(); it++) {
			if (access((_RequestData.fullPath + (*it)).c_str(), F_OK) == 0) {
				_RequestData.fullPath += *it;
				_RequestData.isDir = false;
				break ;
			}
		}
		
		if (it == _RequestData._Config->index.end()) {
			if (!_RequestData._Config->autoindex)
				throw(Code(404));
		}
	}
}

bool						extensionIsCGI(const std::string& fileName, RequestData& _RequestData) {
	size_t dot = fileName.find_last_of('.');
	if (dot == std::string::npos)
		return (false);

	std::string file_ext = fileName.substr(dot);
	
	std::map<std::string, std::string>::iterator it = _RequestData._Config->cgi_ext.find(file_ext);
	if (it != _RequestData._Config->cgi_ext.end())
	{
		_RequestData.cgiIntrepreter = it->second;
		return true;
	}
	return false;
}

void						handleFileResource(RequestData& _RequestData) {
	if (access(_RequestData.fullPath.c_str(), R_OK) != 0)
		throw(Code(403));

	std::string	filename = _RequestData.fullPath.substr(_RequestData.fullPath.find_last_of('/') + 1);

	_RequestData.isCGI = extensionIsCGI(filename, _RequestData);
	if (_RequestData.isCGI)
		_RequestData.scriptName = filename;
	else if (!_RequestData.pathInfo.empty())
		throw(Code(404));
}

void			resolveAbsPath(RequestData& _RequestData) {

	setQueryString(_RequestData);
	setRequestedResourceType(_RequestData);
	if (_RequestData.isDir)
		handleDirectoryResource(_RequestData);
	if (!_RequestData.isDir)
		handleFileResource(_RequestData);
}

void			resolveURI(RequestData& _RequestData) {
	_RequestData.fullPath.clear();

	if (!rootJail(_RequestData.URI))
		throw(Code(403));

	std::cout << "URI: " << _RequestData.URI << std::endl;
	produceAbsPath(_RequestData);
	std::cout << "ABSPATH: " << _RequestData.fullPath << std::endl;
	resolveAbsPath(_RequestData);
	std::cout << "FULLPATH: " << _RequestData.fullPath << std::endl;
	if (_RequestData.isDir && _RequestData.URI[_RequestData.URI.size() - 1] != '/')
		throw(Code(301, "http://" + _RequestData.host + _RequestData.URI + '/'));
}
