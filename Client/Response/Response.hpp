#ifndef RES_HPP
# define RES_HPP

# include "AResponse.hpp"

enum RangeState
{
	NEXT,
	GET
};

typedef	struct								Range
{
	std::pair<int, int>						range;
	std::string								header;
	size_t									rangeLength;
	bool									headerSent;
	Range() : headerSent(false) {}
}											Range;

typedef struct								RangeData
{
	std::vector<Range>						ranges;
	std::vector<Range>::iterator			current;
	std::string								boundary;
	RangeState								rangeState;
	RangeData() : rangeState(NEXT) {}
}											RangeData;

class Response : public AResponse
{
public:
	virtual ~Response();
	Response();
	Response(int &clientSocket, RequestData *data);
	Response& operator=(const Response& rhs);

	inline std::string	buildChunk(const char *data, size_t size) // error
	{
		return (toHex(size) + "\r\n" + std::string(data, size) + "\r\n");
	}

	int		rangeContentLength( void );
	void	parseRangeHeader();
	void	nextRange();
	void	readRange();
	void	range();

	void	readBody();

	void	directoryListing();
	void	initDirList();
	
	void	handleRange( void );

	void	handleGET( void );
	void	handleDELETE( void );
	void	handlePOST( void );

	void	generateHeaders( void );

	int		respond();


private:
	void				(Response::*reader)();
	std::ifstream		bodyFile;
	struct RangeData	rangeData;
	DIR					*dirList;

	std::string		contentType;
	unsigned long	contentLength;

};

#endif