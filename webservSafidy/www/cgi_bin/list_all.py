#!/usr/bin/env python3
import os
import csv
import json


def main():
	# Detect the request method (GET or POST)
	method = os.environ.get("REQUEST_METHOD", "GET")
	csv_file_path = 'data.csv'

	# print("Content-Type: text/html\n")
	print("<html><body>")
	print("<h2>Python CGI Demo</h2>")

	if method == "GET":
		print("<h3>Users :</h3>")
		with open(csv_file_path, mode='r') as csvfile:
			csvreader = csv.reader(csvfile)
			id =  0
			for row in csvreader:
				print(f"<p>id: {id}</p>")
				print(f"<p>name: {row[0]}</p>")
				print(f"<p>age: {row[1]}</p>")
				print(f"<p>comment: {row[2]}</p>")
				print("<hr>")
				id += 1

	print("<a href=\"../html/index.html\">main</a></br>")
	print("<a href=\"../root.html\">Root</a>")
	print("<link rel=\"stylesheet\" href=\"../static/style.css\">")
	print("</body></html>")


if __name__ == "__main__":
	main()
