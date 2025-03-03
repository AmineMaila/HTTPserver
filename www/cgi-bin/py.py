from http.server import BaseHTTPRequestHandler, HTTPServer
import time
import urllib.parse as urlparse

# Simple in-memory session storage
sessions = {}

class SimpleHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/':
            self.send_login_page()
        elif self.path == '/home':
            self.handle_home()
        else:
            self.send_error(404)

    def do_POST(self):
        if self.path == '/login':
            self.handle_login()
        else:
            self.send_error(404)

    def send_login_page(self, error=None):
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        error_html = f'<p style="color:red">{error}</p>' if error else ''
        self.wfile.write(f'''
            <html>
            <body>
                <h1>Login</h1>
                {error_html}
                <form action="/login" method="POST">
                    <input type="text" name="username" placeholder="Username" required>
                    <input type="password" name="password" placeholder="Password" required>
                    <button type="submit">Login</button>
                </form>
            </body>
            </html>
        '''.encode())

    def handle_login(self):
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length).decode()
        params = urlparse.parse_qs(post_data)
        username = params.get('username', [''])[0]
        password = params.get('password', [''])[0]

        # Simple hardcoded credentials check
        if username == 'admin' and password == 'password':
            session_id = str(time.time())
            sessions[session_id] = time.time() + 30  # 30-second expiration
            
            self.send_response(303)
            self.send_header('Location', '/home')
            self.send_header('Set-Cookie', f'session={session_id}')
            self.end_headers()
        else:
            self.send_login_page('Invalid credentials')

    def handle_home(self):
        cookies = self.headers.get('Cookie', '')
        session_id = cookies.split('=')[1] if 'session=' in cookies else None
        
        if session_id and sessions.get(session_id, 0) > time.time():
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            remaining = int(sessions[session_id] - time.time())
            self.wfile.write(f'''
                <html>
                <body>
                    <h1>Welcome admin!</h1>
                    <p>Session expires in: {remaining} seconds</p>
                    <button onclick="location.href='/logout'">Logout</button>
                    <script>
                        setTimeout(() => location.reload(), 1000);
                    </script>
                </body>
                </html>
            '''.encode())
        else:
            self.send_response(303)
            self.send_header('Location', '/')
            self.end_headers()

    def do_GET(self):
        if self.path == '/logout':
            self.handle_logout()
        else:
            super().do_GET()

    def handle_logout(self):
        cookies = self.headers.get('Cookie', '')
        session_id = cookies.split('=')[1] if 'session=' in cookies else None
        if session_id in sessions:
            del sessions[session_id]
        
        self.send_response(303)
        self.send_header('Location', '/')
        self.send_header('Set-Cookie', 'session=; expires=Thu, 01 Jan 1970 00:00:00 GMT')
        self.end_headers()

if __name__ == '__main__':
    server = HTTPServer(('localhost', 8000), SimpleHandler)
    print('Server running on http://localhost:8000')
    server.serve_forever()