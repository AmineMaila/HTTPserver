#!/usr/bin/env python3
import os
import sys
import json
import time
import http.cookies

# Directory to store session data
SESSION_DIR = "/tmp/sessions"

def check_session():
    """
    Checks if a valid session exists.
    Returns:
        tuple: (is_valid, is_expired) - is_valid is a boolean, is_expired indicates if there was an expired session
    """
    is_expired = False
    
    # Check if session cookie exists
    if 'HTTP_COOKIE' in os.environ:
        cookie = http.cookies.SimpleCookie(os.environ['HTTP_COOKIE'])
        if 'session_id' in cookie:
            session_id = cookie['session_id'].value
            
            # Try to load the session file
            session_file = os.path.join(SESSION_DIR, f"{session_id}.json")
            if os.path.exists(session_file):
                try:
                    with open(session_file, 'r') as f:
                        session_data = json.load(f)
                    
                    # Check if session has expired
                    if session_data['expires_at'] > time.time():
                        return True, False
                    else:
                        # Session expired, remove the file
                        os.remove(session_file)
                        is_expired = True
                except Exception:
                    pass
    
    return False, is_expired

# Main execution
is_valid, is_expired = check_session()

if is_valid:
    print("Content-Type: application/json\r\n\r\n", end="")
    print(json.dumps({"valid": True}))
else:
	print("Content-Type: text/html\r\n", end="")
	print("Status: 302 Found\r\n", end="")
	print("Location: http://localhost:8080/session/login.html\r\n\r\n", end="")