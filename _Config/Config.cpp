/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nazouz <nazouz@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/23 13:01:02 by nazouz            #+#    #+#             */
/*   Updated: 2025/02/07 13:36:26 by nazouz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

Config::Config() {}

Config::Config(const std::string& configFileName) {
	if (!openConfigFile(configFileName) || !parseConfigFile() || !constructServers()) {
		ErrorLogger(configFileName + " could not meet the minimum requirements to run Webserv!");
		configFile.close();
		exit(1);
	}

	printServersConfigs();
}

Config::~Config() {
	if (configFile.is_open())
		configFile.close();
	close(logs);
}

void				Config::ErrorLogger(const std::string& error) {
	std::cerr << RED << BOLD << "Webserv: " << error << RESET << std::endl;
}

void				Config::printServersConfigs() {
	for (size_t i = 0; i < Servers.size(); i++) {
		printf("\n\n----------------- SERVER %zu -----------------\n", i + 1);
		printf("LISTEN:\t\t\t%s:%s\n", Servers[i].host.c_str(), Servers[i].port.c_str());
		for (size_t j = 0; j < Servers[i].server_names.size(); j++) {
			printf("SERVER NAME:\t\t%s\n", Servers[i].server_names[j].c_str());
		}
		
		for (std::map<int, std::string>::iterator errorPage = Servers[i].ServerDirectives.error_pages.begin(); errorPage != Servers[i].ServerDirectives.error_pages.end(); errorPage++) {
			printf("ERROR PAGES:\t\t%d | %s\n", errorPage->first, errorPage->second.c_str());
		}
		printf("CLIENT_MBS:\t\t%d\n", Servers[i].ServerDirectives.client_max_body_size);
		printf("ROOT:\t\t\t%s\n", Servers[i].ServerDirectives.root.c_str());
		printf("ALIAS:\t\t\t%s\n", Servers[i].ServerDirectives.alias.c_str());
		
		for (size_t j = 0; j < Servers[i].ServerDirectives.index.size(); j++) {
			printf("INDEX:\t\t\t%s\n", Servers[i].ServerDirectives.index[j].c_str());
		}
		for (size_t j = 0; j < Servers[i].ServerDirectives.index.size(); j++) {
			printf("METHODS:\t\t%s\n", Servers[i].ServerDirectives.methods[j].c_str());
		}
		
		printf("UPLOAD_STORE:\t\t%s\n", Servers[i].ServerDirectives.upload_store.c_str());
		
		if (Servers[i].ServerDirectives.autoindex)
			printf("AUTOINDEX:\t\tON\n");
		else if (!Servers[i].ServerDirectives.autoindex)
			printf("AUTOINDEX:\t\tOFF\n");
		
		printf("REDIRECT:\t\t%d | %s\n", Servers[i].ServerDirectives.redirect.first, Servers[i].ServerDirectives.redirect.second.c_str());
		
		// printf("CGI_EXT:\t\t\t%s\n", Servers[i].ServerDirectives.cgi_ext.c_str());

		std::map<std::string, Directives>::iterator it = Servers[i].Locations.begin();
		for (; it != Servers[i].Locations.end(); ++it) {
			printf("\n----------------- LOCATION %s -----------------\n", it->first.c_str());
			for (std::map<int, std::string>::iterator errorPage = it->second.error_pages.begin(); errorPage != it->second.error_pages.end(); errorPage++) {
				printf("ERROR PAGES:\t\t%d | %s\n", errorPage->first, errorPage->second.c_str());
			}
			printf("CLIENT_MBS:\t\t%d\n", it->second.client_max_body_size);
			printf("ROOT:\t\t\t%s\n", it->second.root.c_str());
			printf("ALIAS:\t\t\t%s\n", it->second.alias.c_str());
			
			for (size_t j = 0; j < it->second.index.size(); j++) {
				printf("INDEX:\t\t\t%s\n", it->second.index[j].c_str());
			}
			for (size_t j = 0; j < it->second.methods.size(); j++) {
				printf("METHODS:\t\t%s\n", it->second.methods[j].c_str());
			}
			
			printf("UPLOAD_STORE:\t\t%s\n", it->second.upload_store.c_str());
			
			if (it->second.autoindex)
				printf("AUTOINDEX:\t\tON\n");
			else if (!it->second.autoindex)
				printf("AUTOINDEX:\t\tOFF\n");

			printf("REDIRECT:\t\t%d | %s\n", it->second.redirect.first, it->second.redirect.second.c_str());
			
			std::map<std::string, std::string>::iterator	ite = it->second.cgi_ext.begin();
			for (; ite != it->second.cgi_ext.end();) {
				printf("CGI_EXT:\t\t\t%s:%s\n", ite->first.c_str(), ite->second.c_str());
				++ite;
			}
			
			printf("----------------- END LOCATION -----------------\n");
		}
		printf("\n----------------- END SERVER -----------------\n");
	}
}

int&										Config::getLogs() {
	return this->logs;
}

std::vector<ServerConfig>&					Config::getServers() {
	return this->Servers;
}

std::ifstream&								Config::getConfigFile() {
	return this->configFile;
}

std::vector<std::string>&					Config::getConfigFileVector() {
	return this->configFileVector;
}

std::vector< std::pair<int, int> >&			Config::getServerBlocksIndexes() {
	return this->serverBlocksIndexes;
}

std::vector< std::pair<int, int> >&			Config::getLocationBlocksIndexes() {
	return this->locationBlocksIndexes;
}
