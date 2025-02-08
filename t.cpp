#include <ctime>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>

#include <iostream>
#include <sstream>
#include <string>

#include <iostream>
void	capitalize(std::string& word)
{
	size_t pos = 0;
	static char alpha[53] = "abcdefghijklmnopqrstuvwxyz";

	while ((pos = word.find_first_of(alpha, pos)) != std::string::npos)
	{
		word[pos++] -= 32;
		pos = word.find_first_not_of(alpha, pos);
	}
}

int main()
{
    std::string key = "hello-wslorrd-a";

	static char alpha[53] = "abcdefghijklmnopqrstuvwxyz";
	std::cout << key.find_first_not_of(alpha, 0) << std::endl;
	std::cout << key << std::endl;
	capitalize(key);
	std::cout << key << std::endl;

    return 0;
}
// void		readCgi(int fd)
// {
// 	char	buf[1024] = {0};
// 	int		bytesRead = read(fd, buf, 1024);
// 	std::cout << "BYTES READ: " << bytesRead << std::endl;
// 	if (bytesRead == -1)
// 	{
// 		std::cerr << "ERRORROROROROOR" << std::endl;
// 	}
// 	if (bytesRead > 0)
// 	{
// 		std::cout << buf;
// 	}
// 	else
// 	{
// 		std::cout << "THE END IS NEVER THE END IS NEVER THE END IS NEVER THE END IS NEVER THE END IS NEVER THE END IS NEVER " << std::endl;
// 	}
// }

// int main()
// {
//     int fd[2];
// 	pipe(fd);
// 	if (fork() == 0)
// 	{
// 		dup2(fd[1], 1);
// 		close(fd[1]);
// 		close(fd[0]);
// 		char *ptr[3];
// 		ptr[0] = "/usr/bin/python3";
// 		ptr[1] = "www/cgi-bin/test.py";
// 		ptr[2] = NULL;
// 		execve(ptr[0], ptr, NULL);
// 	}
// 	close(fd[1]);
// 	while (1)
// 		readCgi(fd[0]);
// }


// int main() {
//     struct timeval start, end;
// 	gettimeofday(&start, NULL);
//     sleep(5);
// 	gettimeofday(&end, NULL);
    
//     double ms = (end.tv_sec - start.tv_sec) * 1000.0;
//     ms += (end.tv_usec - start.tv_usec) / 1000.0;
    
//     std::cout << ms << " milliseconds" << std::endl;
//     return 0;
// }