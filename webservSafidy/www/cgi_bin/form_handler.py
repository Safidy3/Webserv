#!/usr/bin/env python3
# import cgi
import os

print("Content-Type: text/html\n")

print("<html><body>")
print("<h2>Python CGI Demo</h2>")

# Detect the request method (GET or POST)
method = os.environ.get("REQUEST_METHOD", "GET")

if method == "GET":
    query = os.environ.get("QUERY_STRING", "")
    print(f"<p>Request method: GET</p>")
    print(f"<p>Query string: {query}</p>")
    print("<p>Try submitting the POST form below!</p>")

# elif method == "POST":
#     # Read form data
#     form = cgi.FieldStorage()
#     name = form.getvalue("name", "Unknown")
#     age = form.getvalue("age", "Unknown")
#     print(f"<p>Request method: POST</p>")
#     print(f"<p>Hello, {name}! You are {age} years old.</p>")

print("</body></html>")