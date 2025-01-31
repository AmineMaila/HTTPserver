/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmaila <mmaila@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/23 13:01:00 by nazouz            #+#    #+#             */
/*   Updated: 2025/01/30 19:22:20 by mmaila           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <fcntl.h>
#include <unistd.h>
#include <list>
#include <fstream>

#define RESET      "\033[0m"
#define BOLD       "\033[1m"
#define UNDERLINE  "\033[4m"
#define ITALIC     "\033[3m"
#define BLACK      "\033[30m"
#define RED        "\033[31m"
#define GREEN      "\033[32m"
#define YELLOW     "\033[33m"
#define BLUE       "\033[34m"
#define MAGENTA    "\033[35m"
#define CYAN       "\033[36m"
#define WHITE      "\033[37m"
#define BG_BLACK   "\033[40m"
#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN    "\033[46m"
#define BG_WHITE   "\033[47m"


typedef struct												Directives {
	std::vector<std::pair<int, std::string> >				error_pages;
	int														client_max_body_size;
	std::string												root;
	std::string												alias;
	std::vector<std::string>								index;
	std::vector<std::string> 								methods;
	std::string 											upload_store;
	bool													autoindex;
	std::vector<std::string>								redirect; // std::pair<int, std::string>
	std::map<std::string, std::string>						cgi_ext; // std::map<std::string, std::string>
}															Directives;

typedef struct												ServerConfig {
	std::string												host;
	std::string												port;
	std::vector<std::string>								server_names;
	Directives												ServerDirectives;
	std::map<std::string, Directives>						Locations;
}															ServerConfig;

typedef struct												vServerConfigParser {
	std::map<std::string, std::string>						serverDirectives;
	std::vector< std::map<std::string, std::string> >		locationDirectives;
}															vServerConfigParser;


class Config {
	private:
		std::string														configFileName;
		std::ifstream													configFile;
		std::vector<std::string>										configFileVector;
		
		std::vector< std::pair<int, int> >								serverBlocksIndexes;
		std::vector< std::pair<int, int> >								locationBlocksIndexes;

		int																logs;

		std::vector<ServerConfig>										Servers;
		std::vector<vServerConfigParser>								Parser;
		
	public:
		Config();
		Config(const std::string& configFileName);
		~Config();

		bool						openConfigFile();
		// void						fillDefaultDirectives();
		bool						parseConfigFile();
		bool						storeConfigFileInVector();
		
		bool						basicBlocksCountCheck();
		void						fillServerBlocksIndexes();
		std::pair<int, int>			getBlockEndIndex(int blockStart, const std::string startBlockStr);
		bool						validateBlocksIndexes();
		bool						locationBlockIsInsideAServerBlock(int locationStart, int locationEnd);
		
		bool						parseAllServerBlocks();
		bool						parseSingleServerBlock(int start, int end, vServerConfigParser& currentServer);
		bool						addToServerParserServerDirectives(const std::string& key, const std::string& value, std::map<std::string, std::string>& directives);
		bool						validateServerBlockDirectives(std::map<std::string, std::string>& directives);
		bool						parseSingleLocationBlock(int start, int end, vServerConfigParser& currentServer);
		bool						addToServerParserLocationDirectives(const std::string& key, const std::string& value, std::map<std::string, std::string>& directives);
		bool						validateLocationBlockDirectives(std::map<std::string, std::string>& directives);
		bool						isAllowedDirective(const std::string& directive, const std::string& blockType);
		void						Logger(std::string error);

		bool						constructServers();
		void						setDefaultDirectives(std::map<std::string, std::string>& userDirectives);
		bool						fillServerDirectives(std::map<std::string, std::string>& ServerParserDirectives, ServerConfig& newServerConfig);
		std::vector<std::string>	splitStringBySpace(std::string& string);
		bool						fillLocationDirectives(std::map<std::string, std::string>& ServerParserDirectives, Directives& Location, Directives& ServerDirectives);

		void						printServersConfigs();

		int&								getLogs();
		std::vector<ServerConfig>&			getServers();
		std::vector<vServerConfigParser>&	getParser();
		std::ifstream&						getConfigFile();
		std::string&						getConfigFileName();
		std::vector<std::string>&			getConfigFileVector();
		std::vector< std::pair<int, int> >&	getServerBlocksIndexes();
		std::vector< std::pair<int, int> >&	getLocationBlocksIndexes();

};

bool					isValidPort(const std::string& port);
bool					isValidHost(const std::string& host);
bool					isValidServerName(const std::string& serverName);
bool					isValidErrorPage(const std::string& errorPage);
bool					isValidClientMaxBodySize(std::string& client_max_body_size);
bool					isValidPath(const std::string& path);
bool					isValidIndex(const std::string& index);
bool					isValidMethods(const std::string& methods);
bool					isValidRedirect(const std::string& redirect);
bool					isValidAutoIndex(const std::string& autoindex);
bool					isValidCgiPass(const std::string& cgi_pass);
std::string				stringtrim(const std::string& str, const std::string& set);
bool					stringIsDigit(const std::string& str);

#endif