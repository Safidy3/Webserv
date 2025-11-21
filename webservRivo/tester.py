#!/usr/bin/env python3
import os
import csv
import requests
import socket

# Define the server's URL

def get_request(url):
	try:
		response = requests.get(url)
		print(f"\tGET Response: {response.status_code}")
	except requests.exceptions.RequestException as e:
		print(f"GET request failed: {e}")
		return None

def post_request(url, data):
	try:
		response = requests.post(url, data=data)
		print(f"\tPOST Response: {response.status_code}")
	except requests.exceptions.RequestException as e:
		print(f"POST request failed: {e}")
		return None

def delete_request(url):
	try:
		response = requests.delete(url)
		print(f"\tDELETE Response: {response.status_code}")
	except requests.exceptions.RequestException as e:
		print(f"DELETE request failed: {e}")
		return None

def invalid_get_request(url):
	try:
		# Sending an invalid GET request by passing malformed headers
		response = requests.get(url, headers={'Invalid-Header': '\x00'})
		print(f"\tInvalid GET Response: {response.status_code}")
	except requests.exceptions.RequestException as e:
		print(f"Invalid GET request failed: {e}")
		return None

# Function to test the server response
def test_server_response(url):
	print(f"Server: {url}")
	try:
		# Send a GET request to the server
		get_request(url)

		# Send a POST request to the server
		post_request(url, data={'key': 'value'})

		# Send a DELETE request to the server
		delete_request(url)

	except requests.exceptions.RequestException as e:
		print(f"Failed to connect to the server: {e}")


def send_invalid_http_request(host="127.0.0.1", port=8080, invalid_request=b"THIS IS NOT A VALID HTTP REQUEST\r\n\r\n"):
    # Create a raw TCP socket
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))

    # Send the invalid request
    s.sendall(invalid_request)

    # Attempt to receive response (may fail depending on server behavior)
    try:
        response = s.recv(4096)
        print("Response from server:\n", response.decode(errors="replace"))
    except Exception as e:
        print("Error receiving response:", e)

    s.close()

# location = '/downloads'
# location = '/test.py?name=Safidy&lang=en'
# server_url = 'http://localhost:8080' + location
server_url = 'http://localhost:8080'

def main():
	get_request(server_url)
	# post_request(server_url)
	send_invalid_http_request()
	# post_request(server_url, data={'key': 'value'})


if __name__ == "__main__":
	main()



# Run the test
# test_server_response(server_url)
# get_request(server_url)
# post_request(server_url, data={'key': 'value'})
# delete_request(server_url)
# invalid_get_request(server_url)
# print("Testing GET request with query parameters:")

