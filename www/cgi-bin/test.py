#!/usr/bin/env python3
import cgi
import html

# Print HTTP headers
# print("Content-Length: 40\r\n", end="")
# print("Status: 302 Found\r\n", end="")
print("Content-Type: text/html\r\n", end="")
print("HElLO-WslorRd: GoodBye\r\n\r\n", end="")
# print("Location: http://localhost:8080/index.html\r\n\r\n", end="")

# # Get query parameters
# form = cgi.FieldStorage()
# name = form.getvalue('name')

# # Generate HTML response
print("<h1>Hello stranger!</h1>")
# if name:
#     # Escape user input to prevent XSS attacks
#     safe_name = html.escape(name)
#     print(f"<h1>Hello {safe_name}!</h1>")
# else:
#     print("<h1>Hello stranger!</h1>")

# print("</body></html>")

# # print("0\r\n\r\n")