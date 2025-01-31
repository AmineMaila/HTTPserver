#include "../Response.hpp"
#include "../Error.hpp"

void	Response::handleDELETE( void )
{
	if (input.isDir)
	{
		if (rmdir(input.path.c_str()) == -1)
			throw(ErrorPage(500)); // check if 500 is the correct code
	}
	else 
	{
		if (remove(input.path.c_str()) == -1)
			throw(ErrorPage(500)); // check if 500 is the correct code
	}
	nextState = FINISHED;
}