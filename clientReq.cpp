#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

int main() {
    const char *hostname = "localhost";
    const char *port = "8080";  // HTTP default port

    // Step 1: Resolve the server's address
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // Use IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket

    int status = getaddrinfo(hostname, port, &hints, &res);
    if (status != 0) {
        std::cerr << "getaddrinfo failed: " << gai_strerror(status) << std::endl;
        return 1;
    }

    // Step 2: Create the socket
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        std::cerr << "Error creating socket" << std::endl;
        freeaddrinfo(res);
        return 1;
    }

    // Step 3: Connect to the server
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        std::cerr << "Error connecting to server" << std::endl;
        close(sockfd);
        freeaddrinfo(res);
        return 1;
    }

    // Step 4: Send the HTTP GET request
    std::string request = "GET /cgi-bin/test.py HTTP/1.1\r\n"
                          "Host: www.example.com\r\n"
                          "Connection: close\r\n"
                          "\r\n"; // End of headers

    ssize_t sent = send(sockfd, request.c_str(), request.length(), 0);
    if (sent == -1) {
        std::cerr << "Error sending HTTP request" << std::endl;
        close(sockfd);
        freeaddrinfo(res);
        return 1;
    }

    // Step 5: Receive the HTTP response
    char buffer[4096];
    std::string response;
    ssize_t received;
    
    while ((received = recv(sockfd, buffer, sizeof(buffer), 0)) > 0) {
        response.append(buffer, received);
    }

    if (received == -1) {
        std::cerr << "Error receiving response" << std::endl;
    }

    // Step 6: Print out the response
    std::cout << "HTTP Response:\n" << response << std::endl;

    // Step 7: Clean up
    close(sockfd);
    freeaddrinfo(res);
    return 0;
}
