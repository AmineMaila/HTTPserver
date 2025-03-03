#!/usr/bin/env python3
import os
import http.cookies

# Print headers
print("Content-Type: text/html\r\n", end="")

# Clear the session cookie by setting its expiration to the past
cookie = http.cookies.SimpleCookie()
cookie['session_id'] = ""
cookie['session_id']['path'] = '/'
cookie['session_id']['expires'] = 'Thu, 01 Jan 1970 00:00:00 GMT'
print(cookie.output())

# Redirect back to login page
print("Status: 302 Found\r\n", end="")
print("Location: http://localhost:8080/session/login.html\r\n\r\n", end="")