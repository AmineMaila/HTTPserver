#!/usr/bin/env python3
import cgi
import html

print("Content-Type: text/html\r\n\r\n", end="")

# Get query parameters
form = cgi.FieldStorage()
name = form.getvalue('name')

if name:
    # Escape user input to prevent XSS attacks
    safe_name = html.escape(name)
    print(f"<h1>Hello {safe_name}!</h1>")
else:
    print("<h1>Hello stranger!</h1>")

print("</body></html>")