#include "Response.hpp"

void	Response::handleDELETE( void )
{
	if (input.isDir)
	{
		if (rmdir(input.path.c_str()) == -1)
		{
			input.status = 500; // wrong
			generateErrorPage();
			return ;
		}
	}
	else 
	{
		if (remove(input.path.c_str()) == -1)
		{
			input.status = 500; // wrong
			generateErrorPage();
			return ;
		}
	}
	nextState = FINISHED;
}