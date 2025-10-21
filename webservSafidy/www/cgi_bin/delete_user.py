#!/usr/bin/env python3
# import cgi
import os
from urllib.parse import urlparse, parse_qs

def parse_url_query(query):
	query_params = parse_qs(query) # Parse the query string into a dictionary
	return {key: value[0] for key, value in query_params.items()}


# Detect the request method (GET or POST)

def main():
	method = os.environ.get("REQUEST_METHOD", "GET")
	print("Content-Type: text/html\n")
	print("<html><body>")
	print("<h2>Python CGI Delete Demo</h2>")	

	if method == "DELETE":
		print("<p>user deleted successfully !</p>")
	# 	queryString = os.environ.get("QUERY_STRING", "")
	# 	querys = parse_url_query(queryString)
	# 	print(f"<p>User with id: <i>{querys['id']}</i> has been deleted !</p>")
	# else
	# 	print("<h5>WRONG METHOD !</h5>")

	print("<p>by!</p>")
	print("<a href=\"../html/index.html\">main</a></br>")
	print("<a href=\"../root.html\">Root</a>")
	print("</body></html>")

if __name__ == "__main__":
	main()