#!/usr/bin/env python3
import cgi
import html

print("Content-Type: text/html\r\n", end="")
print("Status: 302 Found\r\n", end="")
print("Location: https://google.com\r\n\r\n", end="")
