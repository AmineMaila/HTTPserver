#!/usr/bin/env python3
import cgi
import html

# Print HTTP headers
print("Content-Type: text/html\n\n")

# Get query parameters
form = cgi.FieldStorage()
name = form.getvalue('name')

# Generate HTML response
print("""<html>
<head><title>Greeting Page</title></head>
<body>""")

if name:
    # Escape user input to prevent XSS attacks
    safe_name = html.escape(name)
    print(f"<h1>Hello {safe_name}!</h1>")
else:
    print("<h1>Hello stranger!</h1>")

print("""</body>
</html>""")