#ifndef _ERROR_HPP
# define _ERROR_HPP

#include <iostream>
#include "../../Utils/Helpers.hpp"

class FatalError : public std::exception
{
public:
	FatalError(std::string msg);

	virtual const char *what() const throw();

private:
	std::string msg;
};


class ErrorPage : public std::exception
{
public:
	ErrorPage(int code);

	void generateErrorPage();

private:
	std::string					path;
	std::string					buffer;
	int							status;
	std::map<int, std::string>	description;
};

#endif