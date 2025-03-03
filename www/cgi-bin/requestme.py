import requests
import sys

def fetch_url(url):
	print("fetch_url_begin")
	try:
		print("fetch_url_get_request")
		response = requests.get(url)
		print("fetch_url_raise_error")
		response.raise_for_status()  # Raise an error for bad responses (4xx and 5xx)
		print("fetch_url_print_response")
		print(response.text)
	except requests.exceptions.RequestException as e:
		print("fetch_url_exception")
		print(f"Error: {e}", file=sys.stderr)

if __name__ == "__main__":
		fetch_url("http://localhost:8080/cgi-bin/infinite.py")

