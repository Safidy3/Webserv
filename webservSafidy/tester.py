import requests

# Define the server's URL
location = '/test.py?name=Safidy&lang=en'
server_url = 'http://localhost:8080' + location

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



# Run the test
# test_server_response(server_url)
# get_request(server_url)
# post_request(server_url, data={'key': 'value'})
delete_request(server_url)