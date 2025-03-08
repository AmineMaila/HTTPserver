/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmaila <mmaila@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 18:40:52 by nazouz            #+#    #+#             */
/*   Updated: 2025/03/08 19:31:10 by mmaila           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./HTTPServer/Webserv.hpp"
#include "./Config/Config.hpp"
#include "signal.h"

void	handleINT(int sig)
{
	(void)sig;
	Webserv::stop();
}

int main(int argc, char **argv)
{
	if (argc != 2)
		return (std::cerr << BOLD << "usage: Webserv <config file>" << RESET << std::endl, 1);
	
	signal(SIGINT, handleINT);
	signal(SIGPIPE, SIG_IGN);
	
	Config	conf;
	if (!conf.parse(argv[1]))
		return (1);

	Webserv	Main(conf.getServers());

	if (!Main.initServers())
		return (1);
	Main.run();
	
	return (0);
}