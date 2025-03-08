# Epoll-based HTTP/1.1 Server

A high-performance HTTP/1.1 web server implemented in C++98 using Epoll I/O multiplexing, featuring CGI/1.1 support and a single-threaded, event-driven architecture.

## Features

- **Epoll-based I/O Multiplexing**: Efficient handling of multiple concurrent connections using Linux's Epoll API
- **HTTP/1.1 Compliance**: Supports persistent connections, chunked transfer encoding, and standard methods (GET, POST, DELETE)
- **CGI/1.1 Support**: Execute external programs with proper environment variables and I/O redirection
- **Single-threaded Architecture**: Non-blocking I/O operations managed through Epoll event loop
- **Object-Oriented Design**: Clean separation of server components using modern C++98 practices
- **Configuration File Support**: Flexible server configuration through human-readable config files

## Building the Project

### Requirements
- Linux kernel 2.6+ (Epoll support)
- C++98-compatible compiler (g++ recommended)
- GNU Make

### Compilation
```bash
$ make clean && make
