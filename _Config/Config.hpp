/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nazouz <nazouz@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/23 13:01:00 by nazouz            #+#    #+#             */
/*   Updated: 2025/02/07 13:02:20 by nazouz           ###   ########.fr       */
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
#include <algorithm>
#include <limits.h>

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


typedef struct								Directives{
	std::map<int, std::string>				error_pages;
	int										client_max_body_size;
	std::string								root;
	std::string								alias;
	std::vector<std::string>				index;
	std::vector<std::string> 				methods;
	std::string 							upload_store;
	bool									autoindex;
	std::pair<int, std::string>				redirect;
	std::map<std::string, std::string>		cgi_ext;

	// default values for every directives
	Directives() {
		// error_pages.push_back(std::make_pair(404, "/home/nazouz/Desktop/Webserv/Errors/400.html"));
		client_max_body_size = INT_MAX;
		root = "/home/mmaila/Desktop/SERV/www/";
		index.push_back("index.html");
		index.push_back("index.py");
		index.push_back("index.php");
		methods.push_back("GET");
		methods.push_back("POST");
		methods.push_back("DELETE");
		upload_store = "/home/nazouz/goinfre/WebservUpload";
		autoindex = false;
		cgi_ext[".sh"] = "/usr/bin/sh";
		cgi_ext[".py"] = "/usr/bin/python3";
		cgi_ext[".php"] = "/usr/bin/php";
	}
}															Directives;

typedef struct												ServerConfig {
	std::string												host;
	std::string												port;
	std::vector<std::string>								server_names;
	Directives												ServerDirectives;
	std::map<std::string, Directives>						Locations;

	// default values for server only directives
	ServerConfig() {
		host = "127.0.0.1";
		port = "8080";
		server_names.push_back("nazouz.com");
		server_names.push_back("mmaila.com");
	}
}															ServerConfig;

class Config {
	private:
		std::ifstream													configFile;
		std::vector<std::string>										configFileVector;
		
		std::vector< std::pair<int, int> >								serverBlocksIndexes;
		std::vector< std::pair<int, int> >								locationBlocksIndexes;

		int																logs;

		std::vector<ServerConfig>										Servers;
		
	public:
		Config();
		Config(const std::string& configFileName);
		~Config();

		bool						openConfigFile(const std::string& configFileName);
		bool						parseConfigFile();
		bool						storeConfigFileInVector();
		
		bool						basicBlocksCountCheck();
		void						fillServerBlocksIndexes();
		std::pair<int, int>			getBlockEndIndex(int blockStart, const std::string startBlockStr);
		bool						validateBlocksIndexes();
		bool						locationBlockIsInsideAServerBlock(int locationStart, int locationEnd);
		
		bool						parseSingleServerBlock(int start, int end, ServerConfig& currentServer);
		bool						fillServerBlockDirectives(std::string& key, std::string& value, std::vector<std::string>& alreadyParsed, ServerConfig& currentServer);
		bool						fillLocationBlockDirectives(std::string& key, std::string& value, std::vector<std::string>& alreadyParsed, Directives& toFill);
		bool						parseSingleLocationBlock(int start, int end, ServerConfig& currentServer);
		void						ErrorLogger(const std::string& error);

		bool						constructServers();


		bool						isValidPort(const std::string& port, ServerConfig& currentServer);
		bool						isValidHost(const std::string& host, ServerConfig& currentServer);
		bool						isValidServerName(const std::string& serverName, ServerConfig& currentServer);
		bool						isValidErrorPage(const std::string& errorPage, Directives& toFill);
		bool						isValidClientMaxBodySize(const std::string& client_max_body_size, Directives& toFill);
		bool						isValidRoot(const std::string& root, Directives& toFill);
		bool						isValidUploadStore(const std::string& upload_store, Directives& toFill);
		bool						isValidLocation(const std::string& location);
		bool						isValidAlias(const std::string& alias, Directives& toFill);
		bool						isValidIndex(const std::string& index, Directives& toFill);
		bool						isValidMethods(const std::string& methods, Directives& toFill);
		bool						isValidRedirect(const std::string& redirect, Directives& toFill);
		bool						isValidAutoIndex(const std::string& autoindex, Directives& toFill);
		bool						isValidCgiExt(const std::string& cgi_pass, Directives& toFill);
		void						printServersConfigs();

		int&								getLogs();
		std::vector<ServerConfig>&			getServers();
		// std::vector<vServerConfigParser>&	getParser();
		std::ifstream&						getConfigFile();
		// std::string&						getConfigFileName();
		std::vector<std::string>&			getConfigFileVector();
		std::vector< std::pair<int, int> >&	getServerBlocksIndexes();
		std::vector< std::pair<int, int> >&	getLocationBlocksIndexes();

};

std::string				stringtrim(const std::string& str, const std::string& set);
bool					stringIsDigit(const std::string& str);

#endif