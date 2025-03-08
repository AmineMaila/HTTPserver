#!/usr/bin/env python3
import cgi
import html

print("Content-Type: text/html\r\n\r\n", end="")

form = cgi.FieldStorage()
name = form.getvalue('name')

print("""<!DOCTYPE html>
<html>
<head>
    <title>Hello Page</title>
</head>
<body>""")

if name:
    safe_name = html.escape(name) # xss protection
    print(f"    <h1>Hello {safe_name}!</h1>")
else:
    print("    <h1>Hello stranger!</h1>")

print("""</body>
</html>""")