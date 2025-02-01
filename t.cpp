#include <ctime>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>

int main() {
    struct timeval start, end;
	gettimeofday(&start, NULL);
    sleep(5);
	gettimeofday(&end, NULL);
    
    double ms = (end.tv_sec - start.tv_sec) * 1000.0;
    ms += (end.tv_usec - start.tv_usec) / 1000.0;
    
    std::cout << ms << " milliseconds" << std::endl;
    return 0;
}