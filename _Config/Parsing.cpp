/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nazouz <nazouz@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/24 20:20:43 by nazouz            #+#    #+#             */
/*   Updated: 2025/03/01 18:45:17 by nazouz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

bool				Config::openConfigFile(const std::string& configFileName) {
	size_t				pos = configFileName.find(".conf");
	if (pos == 0 || pos == std::string::npos || configFileName.substr(pos) != ".conf")
		return (ErrorLogger("Invalid config file extention!"), false);
	
	configFile.open(configFileName.c_str(), std::ios::in);
	if (configFile.is_open())
		return true;
	return (ErrorLogger("can't open config file!"), false);
}

bool				Config::parseConfigFile() {
	storeConfigFileInVector();
	if (!basicBlocksCountCheck())
		return (ErrorLogger("Invalid Server/Location blocks syntax!"), false);
	fillServerBlocksIndexes();
	if (!validateBlocksIndexes())
		return (ErrorLogger("Invalid Server/Location blocks syntax!"), false);
	return true;
}

bool				Config::storeConfigFileInVector() {
	size_t				pos;
	std::string			line;

	while (std::getline(configFile, line)) {
		if (line.empty())
			continue;
		line = stringtrim(line, " \t");
		if (line.empty())
			continue;
		if (line[0] == '#')
			continue;
		if ((pos = line.find('#')) != std::string::npos)
			line = line.substr(0, pos);
		configFileVector.push_back(line);
		if (configFile.eof())
			break;
	}
	return true;
}

bool				Config::basicBlocksCountCheck() {
	int					serverStart = 0;
	int					serverEnd = 0;
	int					locationStart = 0;
	int					locationEnd = 0;
	
	for (size_t i = 0; i < configFileVector.size(); i++) {
		if (configFileVector[i] == "[SERVER]")
			serverStart++;
		else if (configFileVector[i] == "[/SERVER]")
			serverEnd++;
		else if (configFileVector[i] == "[LOCATION]")
			locationStart++;
		else if (configFileVector[i] == "[/LOCATION]")
			locationEnd++;
	}
	if (!serverStart || !serverEnd)
		return false;
	if ((serverStart != serverEnd) || (locationStart != locationEnd))
		return false;
	return true;
}

std::pair<int, int>		Config::getBlockEndIndex(int blockStart, const std::string startBlockStr) {
	size_t					i = blockStart;
	std::string				endBlock;
	std::pair<int, int>		toReturn(blockStart, -1);
	
	if (startBlockStr == "[SERVER]")
		endBlock = "[/SERVER]";
	else if (startBlockStr == "[LOCATION]")
		endBlock = "[/LOCATION]";

	for (; i < configFileVector.size(); i++) {
		if (configFileVector[i] == endBlock) {
			toReturn = std::make_pair(blockStart, i);
			return toReturn;
		}
	}
	return toReturn;
}

void				Config::fillServerBlocksIndexes() {
	for (size_t i = 0; i < configFileVector.size(); i++) {
		if (configFileVector[i] == "[SERVER]")
			serverBlocksIndexes.push_back(getBlockEndIndex(i, "[SERVER]"));
		else if (configFileVector[i] == "[LOCATION]")
			locationBlocksIndexes.push_back(getBlockEndIndex(i, "[LOCATION]"));
	}
}

bool				Config::locationBlockIsInsideAServerBlock(int locationStart, int locationEnd) {
	// std::pair<int, int>		matchingServerBlock;
	for (size_t i = 0; i < serverBlocksIndexes.size(); i++) {
		if (serverBlocksIndexes[i].first < locationStart && serverBlocksIndexes[i].second > locationEnd)
			return true;
	}
	return false;
}

bool				Config::validateBlocksIndexes() {
	int			a = 0, b = 0, c = 0, d = 0;
	for (size_t i = 0; i < serverBlocksIndexes.size() - 1; i++) {
		a = serverBlocksIndexes[i].first;
		b = serverBlocksIndexes[i].second;
		c = serverBlocksIndexes[i + 1].first;
		d = serverBlocksIndexes[i + 1].second;
		if (b + 1 != c)
			return false;
		if (a == -1 || b == -1 || c == -1 || d == -1)
			return false;
		if (a < b && b < c && c < d)
			continue;
		return false;
	}
	if (!locationBlocksIndexes.size())
		return true;
	for (int i = 0; i < static_cast<int>(locationBlocksIndexes.size() - 1); i++) {
		a = locationBlocksIndexes[i].first;
		b = locationBlocksIndexes[i].second;
		c = locationBlocksIndexes[i + 1].first;
		d = locationBlocksIndexes[i + 1].second;
		if (a == -1 || b == -1 || c == -1 || d == -1)
			return false;
		if (!(a < b && b < c && c < d))
			return false;
		if (locationBlockIsInsideAServerBlock(a, b))
			continue;
		return false;
	}
	a = locationBlocksIndexes[locationBlocksIndexes.size() - 1].first;
	b = locationBlocksIndexes[locationBlocksIndexes.size() - 1].second;
	if (!locationBlockIsInsideAServerBlock(a, b))
		return false;
	return true;
}
