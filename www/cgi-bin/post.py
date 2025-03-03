#!/usr/bin/env python3
import cgi
import cgitb
import sys
import json
import os

# Enable debugging (useful for development)
cgitb.enable()


# Set the content type to JSON
print("Content-Type: application/json\r\n\r\n", end="")

# Define the file where the JSON data will be saved
DATA_FILE = 'data.json'

try:
    # Get the request method
    method = os.environ.get('REQUEST_METHOD', '')

    if method == 'POST':
        # Read the raw data from the request body
        content_length = int(os.environ.get('CONTENT_LENGTH', 0))
        post_data = sys.stdin.read(content_length)

        # Parse the JSON data
        json_data = json.loads(post_data)

        # Save the JSON data to a file
        with open(DATA_FILE, 'w') as f:
            json.dump(json_data, f, indent=4)

        # Prepare the response
        response = {
            "status": "success",
            "message": "Data saved successfully",
            "elements": list(json_data.keys())
        }

        # Send the response as JSON
        print(json.dumps(response))
    else:
        # If the request method is not POST, return an error
        response = {
            "status": "error",
            "message": "Only POST requests are supported"
        }
        print(json.dumps(response))

except Exception as e:
    # Handle any exceptions and return an error message
    response = {
        "status": "error",
        "message": str(e)
    }
    print(json.dumps(response))