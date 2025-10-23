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

	if method == "POST":
		print("<h2>Python CGI Demo</h2>")	
		queryString = os.environ.get("QUERY_STRING", "")
		querys = parse_url_query(queryString)

		print(f"<p>User <i>{querys['name']}</i> has been created !</p>")

		print("<p>by!</p>")
		print("<a href=\"../html/index.html\">main</a></br>")
		print("<a href=\"../root.html\">Root</a>")

	print("<link rel=\"stylesheet\" href=\"../static/style.css\">")
	print("</body></html>")

if __name__ == "__main__":
	main()