#ifndef _ERROR_HPP
# define _ERROR_HPP

#include "AResponse.hpp"

class ErrorPage : public AResponse
{
public:
	virtual ~ErrorPage();
	ErrorPage(Code e, int& socket, RequestData	*data);

	void	readBody( void );
	void	generateHeaders( void );
	int		respond( void );
	std::ostream& operator<<(std::ostream& os) const;
	
private:
	std::ifstream	bodyFile;
	Code			status;
};

#endif