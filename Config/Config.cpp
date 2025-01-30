/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmaila <mmaila@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/23 13:01:02 by nazouz            #+#    #+#             */
/*   Updated: 2025/01/19 21:02:02 by mmaila           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include <sstream>

Config::Config() {
	
}

Config::Config(const std::string& configFileName) {
	this->configFileName = configFileName;
	
	logs = open("Logs/webserv.log", O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (logs == -1)
		std::cerr << BOLD << "Webserv: can't create error.log!" << RESET << std::endl;

	// if (!openConfigFile())
	// 	return ;
	
	// fillDefaultDirectives();
}

Config::~Config() {
	if (configFile.is_open())
		configFile.close();
	close(logs);
}

// void				Config::fillDefaultDirectives() {
// 	defaultServerDirectives["host"] = "0.0.0.0";
// 	defaultServerDirectives["port"] = "80";
// 	defaultServerDirectives["server_name"] = "none";
// 	defaultServerDirectives["error_page"] = "none";
// 	defaultServerDirectives["client_max_body_size"] = "none";
// 	defaultServerDirectives["root"] = "/home/mmaila/Desktop/SERV/www";
// 	defaultServerDirectives["index"] = "index.html";
// 	defaultServerDirectives["methods"] = "GET POST DELETE";
// 	defaultServerDirectives["upload_store"] = "none";
// 	defaultServerDirectives["redirect"] = "none";
// 	defaultServerDirectives["autoindex"] = "off";
// 	defaultServerDirectives["cgi_pass"] = "none";

// 	defaultLocationDirectives["location"] = "none";
// 	defaultLocationDirectives["root"] = "none";
// 	defaultLocationDirectives["index"] = "index.html";
// 	defaultLocationDirectives["methods"] = "GET POST DELETE";
// 	defaultLocationDirectives["upload_store"] = "none";
// 	defaultLocationDirectives["redirect"] = "none";
// 	defaultLocationDirectives["autoindex"] = "off";
// 	defaultLocationDirectives["cgi_pass"] = "none";
// 	defaultLocationDirectives["error_page"] = "none";
// 	defaultLocationDirectives["client_max_body_size"] = "none";
// }

void				Config::Logger(std::string error) {
	std::cerr << BOLD << RED << "Webserv: see error.log" << RESET << std::endl;
	error += "\n";
	write(logs, error.c_str(), error.length());
}

// void				Config::printServersConfigs() {
// 	// printf("printServersConfigs()\n");
// 	for (size_t i = 0; i < Servers.size(); i++) {
// 		printf("\n\n----------------- SERVER %zu -----------------\n", i);
// 		printf("LISTEN:\t\t\t%s:%d\n", Servers[i].host.c_str(), Servers[i].port);
// 		for (size_t j = 0; j < Servers[i].server_name.size(); j++) {
// 			printf("SERVER NAME:\t\t\t%s\n", Servers[i].server_name[j].c_str());
// 		}
// 		for (size_t j = 0; j < Servers[i].locations.size(); j++) {
// 			printf("----------------- LOCATION %s -----------------\n", Servers[i].locations[j].location.c_str());
// 			printf("ROOT:\t\t\t%s\n", Servers[i].locations[j].root.c_str());
// 			for (size_t k = 0; k < Servers[i].locations[j].index.size(); k++) {
// 				printf("INDEX %zu:\t\t\t%s\n", k, Servers[i].locations[j].index[k].c_str());
// 			}
// 			for (size_t k = 0; k < Servers[i].locations[j].methods.size(); k++) {
// 				printf("METHOD %zu:\t\t\t%s\n", k, Servers[i].locations[j].methods[k].c_str());
// 			}
// 			printf("UPLOAD:\t\t\t%s\n", Servers[i].locations[j].upload_store.c_str());
// 			for (size_t k = 0; k < Servers[i].locations[j].redirect.size(); k++) {
// 				printf("REDIRECT %zu:\t\t\t%s\n", k, Servers[i].locations[j].redirect[k].c_str());
// 			}
// 			printf("AUTOINDEX:\t\t\t%s\n", Servers[i].locations[j].autoindex.c_str());
// 			printf("CGI:\t\t\t%s\n", Servers[i].locations[j].cgi_pass.c_str());
// 			for (size_t k = 0; k < Servers[i].locations[j].error_page.size(); k++) {
// 				printf("ERROR_PAGE %zu:\t\t\t%s\n", k, Servers[i].locations[j].error_page[k].c_str());
// 			}
// 			printf("CLIENT_MBS:\t\t\t%s\n", Servers[i].locations[j].client_max_body_size.c_str());
// 			printf("----------------- END LOCATION -----------------\n");
// 		}
// 		printf("----------------- END SERVER -----------------\n");
// 	}
// }

int&										Config::getLogs() {
	return this->logs;
}

std::vector<vServerConfig>&					Config::getServers() {
	return this->Servers;
}

std::vector<vServerConfigParser>&			Config::getParser() {
	return this->Parser;
}

std::ifstream&								Config::getConfigFile() {
	return this->configFile;
}

std::string&								Config::getConfigFileName() {
	return this->configFileName;
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
