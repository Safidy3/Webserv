import os
from urllib.parse import urlparse, parse_qs

def parse_url_query(query):
	query_params = parse_qs(query) # Parse the query string into a dictionary
	return {key: value[0] for key, value in query_params.items()}

print("Content-Type: text/html\n")
print("<html><body>")
print("<h2>Python CGI Demo</h2>")

# Detect the request method (GET or POST)
method = os.environ.get("REQUEST_METHOD", "GET")

if method == "GET":
	print("<p>GET request :</p>")
	queryString = os.environ.get("QUERY_STRING", "")
	querys = parse_url_query(queryString)
	print(f"<p>your name is <i>{querys['name']}</i> and you are <i>{querys['age']}</i></p>")

elif method == "POST":
	print("<p>POST request :</p>")
	queryString = os.environ.get("QUERY_STRING", "")
	querys = parse_url_query(queryString)
	print(f"<p>your name is <i>{querys['name']}</i> and you are <i>{querys['age']}</i></p>")

print("<p>by!</p>")
print("<a href=\"../html/index.html\">main</a></br>")
print("<a href=\"../root.html\">Root</a>")
print("</body></html>")