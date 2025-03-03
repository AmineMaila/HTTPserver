#!/usr/bin/env python3
import cgi
import os
import time
import http.cookies
import random
import string
import json
import sys

# Directory to store session data
SESSION_DIR = "/tmp/sessions"

# Ensure the session directory exists
if not os.path.exists(SESSION_DIR):
    try:
        os.makedirs(SESSION_DIR)
    except Exception as e:
        print("Content-Type: text/html\r\n\r\n", end="")
        print(f"<html><body><h1>Error</h1><p>Could not create session directory: {e}</p></body></html>")
        sys.exit(1)

# Function to generate a random session ID
def generate_session_id(length=32):
    characters = string.ascii_letters + string.digits
    return ''.join(random.choice(characters) for _ in range(length))

# Function to create a new session
def create_session(username):
    session_id = generate_session_id()
    session_data = {
        'username': username,
        'created_at': time.time(),
        'expires_at': time.time() + 30  # 30 seconds from now
    }
    
    # Save session to file
    with open(os.path.join(SESSION_DIR, f"{session_id}.json"), 'w') as f:
        json.dump(session_data, f)
    
    return session_id, session_data

# Function to get an existing session
def get_session(session_id):
    try:
        session_file = os.path.join(SESSION_DIR, f"{session_id}.json")
        if os.path.exists(session_file):
            with open(session_file, 'r') as f:
                session_data = json.load(f)
                
            # Check if session has expired
            if session_data['expires_at'] <= time.time():
                os.remove(session_file)  # Remove expired session
                return None
            return session_data
        return None
    except Exception:
        return None

# Function to verify login credentials (for demo purposes)
def verify_credentials(username, password):
    # In a real application, you would verify against a database
    # For this demo, we'll accept any username with password "password"
    return password == "password"

# Main function to handle the login process
def main():
    # Create a cookie object
    cookie = http.cookies.SimpleCookie()
    
    # Check if session cookie exists
    if 'HTTP_COOKIE' in os.environ:
        existing_cookie = http.cookies.SimpleCookie(os.environ['HTTP_COOKIE'])
        if 'session_id' in existing_cookie:
            session_id = existing_cookie['session_id'].value
            session = get_session(session_id)
            
            if session:
                # Valid session exists, redirect to the dashboard or homepage
                print("Content-Type: text/html\r\n", end="")
                print("Status: 302 Found\r\n", end="")
                print("Location: http://localhost:8080/session/dashboard.html\r\n\r\n", end="")
                return
    
    # No valid session, process the login form
    form = cgi.FieldStorage()
    
    # Check if form data is submitted
    if "username" in form and "password" in form:
        username = form.getvalue("username")
        password = form.getvalue("password")
        
        if verify_credentials(username, password):
            # Create a new session
            session_id, session_data = create_session(username)
            
            # Set the session cookie with expiration
            cookie['session_id'] = session_id
            cookie['session_id']['path'] = '/'
            cookie['session_id']['max-age'] = 30  # 30 seconds
            cookie['session_id']['HttpOnly'] = True
            
            # Redirect to the dashboard or homepage
            print("Content-Type: text/html\r\n", end="")
            print(cookie.output(), end="\r\n")
            print("Status: 302 Found\r\n", end="")
            print("Location: http://localhost:8080/session/dashboard.html\r\n\r\n", end="")
        else:
            # Invalid credentials
            print("Content-Type: text/html\r\n\r\n", end="")
            print("""
            <html>
            <head>
                <title>Login Failed</title>
                <meta http-equiv="refresh" content="3;url=/login.html">
                <style>
                    body {
                        font-family: Arial, sans-serif;
                        display: flex;
                        justify-content: center;
                        align-items: center;
                        height: 100vh;
                        margin: 0;
                        background-color: #f0f0f0;
                    }
                    .message {
                        background-color: white;
                        padding: 2rem;
                        border-radius: 8px;
                        box-shadow: 0 0 10px rgba(0,0,0,0.1);
                        text-align: center;
                    }
                </style>
            </head>
            <body>
                <div class="message">
                    <h2>Login Failed</h2>
                    <p>Invalid username or password.</p>
                    <p>Redirecting back to login page...</p>
                </div>
            </body>
            </html>
            """)
    else:
        # No form data, redirect to login page
        print("Content-Type: text/html\r\n", end="")
        print("Status: 302 Found\r\n", end="")
        print("Location: http://localhost:8080/session/login.html\r\n\r\n", end="")

if __name__ == "__main__":
    main()