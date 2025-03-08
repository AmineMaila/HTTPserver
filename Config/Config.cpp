/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmaila <mmaila@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/23 13:01:02 by nazouz            #+#    #+#             */
/*   Updated: 2025/03/07 18:24:18 by mmaila           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

Config::Config() {}

bool	Config::parse(const std::string& configFileName)
{
	if (!openConfigFile(configFileName) || !parseConfigFile() || !constructServers()) {
		ErrorLogger(configFileName + " could not meet the minimum requirements to run Webserv!");
		configFile.close();
		return false;
	}
	configFile.close();
	return true;
}

Config::~Config()
{
	if (configFile.is_open())
		configFile.close();
}

void	Config::ErrorLogger(const std::string& error)
{
	std::cerr << RED << BOLD << "Webserv: " << error << RESET << std::endl;
}

std::vector<ServerConfig>&	Config::getServers()
{
	return this->Servers;
}

std::ifstream&	Config::getConfigFile()
{
	return this->configFile;
}

std::vector<std::string>&	Config::getConfigFileVector()
{
	return this->configFileVector;
}

std::vector< std::pair<int, int> >&	Config::getServerBlocksIndexes()
{
	return this->serverBlocksIndexes;
}

std::vector< std::pair<int, int> >&	Config::getLocationBlocksIndexes()
{
	return this->locationBlocksIndexes;
}
