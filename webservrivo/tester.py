#!/usr/bin/env python3
import os
import csv
import requests

# Define the server's URL
location = '/downloads'
location = '/test.py?name=Safidy&lang=en'
server_url = 'http://localhost:8080'

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



# def main():
# 	# Detect the request method (GET or POST)
# 	csv_file_path = 'data.csv'

# 	print("Content-Type: text/html\n")
# 	print("<html><body>")
# 	print("<h2>Python CGI Demo</h2>")

# 	print("<h3>Users :</h3>")
# 	with open(csv_file_path, mode='r') as csvfile:
# 		csvreader = csv.reader(csvfile)
# 		for row in csvreader:
# 			print(f"<p>name: {row[0]}</p>")
# 			print(f"<p>age: {row[1]}</p>")
# 			print(f"<p>comment: {row[2]}</p>")
# 			print("<hr>")

# 	print("<a href=\"../html/index.html\">main</a></br>")
# 	print("<a href=\"../root.html\">Root</a>")
# 	print("</body></html>")


def main():
	post_request(server_url, data={'key': 'value'})
	get_request(server_url)
	delete_request(server_url)


if __name__ == "__main__":
	main()




# Run the test
# test_server_response(server_url)
# get_request(server_url)
# post_request(server_url, data={'key': 'value'})
# delete_request(server_url)
# invalid_get_request(server_url)
# print("Testing GET request with query parameters:")