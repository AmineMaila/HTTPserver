# HTTP Server

A high-performance HTTP/1.1 compliant server implemented in C++98 with Epoll I/O multiplexing for efficient connection handling. This server supports various HTTP features along with CGI/1.1 for dynamic content processing.

## HTTP in Action

### Example HTTP Request
```
GET /index.html HTTP/1.1
Host: example.com
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36
Accept: text/html,application/xhtml+xml,application/xml
Accept-Language: en-US,en;q=0.9
Connection: keep-alive
```

### Example HTTP Response
```
HTTP/1.1 200 OK
Server: Webserv/1.0
Date: Sun, 09 Mar 2025 12:00:00 GMT
Content-Type: text/html
Content-Length: 348
Connection: keep-alive

<!DOCTYPE html>
<html>
<head>
    <title>Welcome to Webserv</title>
</head>
<body>
    <h1>Hello, World!</h1>
    <p>Your HTTP server is working correctly.</p>
</body>
</html>
```

## Features

- HTTP/1.1 compliance
- Non-blocking I/O with Epoll multiplexing
- Object-oriented architecture
- Virtual server hosting support
- Custom configuration file format
- CGI/1.1 support for dynamic content
- Redirection and custom error pages
- Directory listing (autoindex)
- File uploads
- Method restrictions

## Building

The server can be built using the provided Makefile.

```bash
# Clone the repository
git clone https://github.com/AmineMaila/HTTPserver.git
cd HTTPserver

# Build the project
make

# Clean object files
make clean

# Clean object files and executable
make fclean

# Rebuild
make re
```

## Usage

To start the server, run the executable with a configuration file:

```bash
./webserv [config_file]
```

Example:
```bash
./webserv default.conf
```

## Configuration

The server uses a custom configuration format with sections for server and location blocks. Below is an example of a basic configuration file:

```ini
[SERVER]
  host = 0.0.0.0
  port = 8080
  server_name = mmaila.com
  root = /home/mmaila/Desktop/HTTPserver/www/
  [LOCATION]
    location = /cgi-bin
    index = querystring.py
    cgi_ext = .py:/usr/bin/python3
  [/LOCATION]

  [LOCATION]
    location = /mmaila
    redirect = 302 https://github.com/AmineMaila
  [/LOCATION]
[/SERVER]
```

### Supported Directives

| Directive | Description | Example |
|-----------|-------------|---------|
| `host` | IP address to bind to | `host = 0.0.0.0` |
| `port` | Port to listen on | `port = 8080` |
| `server_name` | Server name for virtual hosting | `server_name = mmaila.com` |
| `root` | Root directory for serving files (mandatory) | `root = /var/www/html/` |
| `index` | Default file to serve for directory requests | `index = index.html` |
| `redirect` | HTTP redirection with status code | `redirect = 302 https://github.com/AmineMaila` |
| `alias` | Map location to different directory | `alias = /var/www/images/` |
| `cgi_ext` | Map file extensions to CGI interpreters | `cgi_ext = .py:/usr/bin/python3` |
| `error_page` | Custom error pages | `error_page = 404 403 400 /error.html` |
| `client_max_body_size` | Maximum upload size in bytes | `client_max_body_size = 1024K` |
| `upload_store` | Directory for file uploads | `upload_store = /var/www/uploads` |
| `autoindex` | Enable directory listing | `autoindex = on` |
| `methods` | Allowed HTTP methods | `methods = GET POST DELETE` |
| `location` | URL path for location block | `location = /api` |

## Architecture Overview

The server is built using an object-oriented approach, with the main components being:

- **Webserv**: initializes the servers and routes epoll events
- **ClientHandler**: Encapsulates client connection state
- **ServerHandler**: Manages the listening socket
- **CGIHandler**: Executes CGI script in a child process
- **IEventHandler**: Interface that all connection related classes inherit from

### Epoll Event Loop

The heart of the server is an efficient Epoll-based event loop that handles I/O multiplexing for non-blocking connection processing. Below is a simplified representation of the event loop:

```cpp
void Webserv::run() {
    struct epoll_event events[MAX_EVENTS];
    
    while (running) {
        // Wait for events with a timeout
        int eventCount = epoll_wait(epoll_fd, events, MAX_EVENTS, EPOLL_TIMEOUT);
        
        // Process each event
        for (int i = 0; i < eventCount; i++) {
            // Get the handler object from the event data
            EventHandler *handler = static_cast<EventHandler *>(events[i].data.ptr);
            
            try {
                // Polymorphic call to the appropriate handler method
                handler->handleEvent(events[i].events);
            } catch (Disconnect& e) {
                // Handle client disconnection gracefully
                std::cerr << YELLOW << e.what() << RESET << std::endl;
                cleanup(handler);
            }
        }
        
        // Check for and remove inactive clients
        clientTimeout();
    }
}
```

### IEventHandler Interface
```cpp
class IEventHandler
{
public:
    // Pure virtual destructor ensures proper cleanup of derived classes
    virtual ~IEventHandler() {}
    
    // Main method called by the event loop when an event occurs
    virtual void handleEvent(uint32_t events) = 0;
    
    // Returns the file descriptor this handler is responsible for
    virtual int getFd() const = 0;

    // Pointer to the Main class to control the handler's interactions with Epoll
    Webserv	*HTTPserver;
}
```
The core of the server's design is the IEventHandler interface, which defines the contract for all event handlers:

## This implementation demonstrates:

- Using a pointer to the handler object in the event data for polymorphic dispatch
- Exception handling for clean client disconnection
- Periodic timeout checking to manage inactive connections
- Clean object-oriented design principles

The event loop efficiently handles multiple connections without spawning threads, making it highly scalable for concurrent connections.

## CGI Processing

When a CGI request is detected, the server creates a child process to execute the CGI script, passing environment variables and handling input/output according to the CGI/1.1 specification.

## Performance Considerations

This server is designed for high performance through:

- Non-blocking I/O operations
- Efficient memory management
- Minimal data copying
- Connection timeouts to prevent resource exhaustion

## Limitations

- No native HTTPS support
- No threading (single-process model)
- Limited to C++98 features

## License

This project is licensed under the [MIT License](LICENSE) - see the [LICENSE](LICENSE) file for details

## Collaborator

[nazouz](https://github.com/xezzuz)
