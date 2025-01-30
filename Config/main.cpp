/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nazouz <nazouz@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/23 16:34:22 by nazouz            #+#    #+#             */
/*   Updated: 2024/11/27 12:11:46 by nazouz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
// #include <sstream>
// #include <iostream>

int main(int argc, char **argv) {
	if (argc < 2)
		return 1;
	
	Config		configfile(argv[1]);

	if (!configfile.parseConfigFile()) {
		std::cerr << BOLD << RED << "Webserv: " << argv[1] << " doesn't meet the requirements to run Webserv!" << std::endl;
		return 1;
	}
	std::cout << BOLD << "Webserv: " << argv[1] << " is valid to run Webserv!" << std::endl;

	configfile.constructServers();
	configfile.printServersConfigs();
	
	return 0;
}
