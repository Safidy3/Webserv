#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "../webserv.hpp"

class HTTPResponse
{
private:
	int									statusCode_;
	std::string							reasonPhrase_;
	std::string							httpVersion_;
	std::map<std::string, std::string>	headers_;
	std::string							body_;

	std::string getReasonPhrase(int code) const
	{
		switch (code)
		{
			case 200: return "OK";
			case 201: return "Created";
			case 204: return "No Content";
			case 301: return "Moved Permanently";
			case 302: return "Found";
			case 304: return "Not Modified";
			case 400: return "Bad Request";
			case 401: return "Unauthorized";
			case 403: return "Forbidden";
			case 404: return "Not Found";
			case 500: return "Internal Server Error";
			case 502: return "Bad Gateway";
			case 503: return "Service Unavailable";
			default : return "Unknown";
		}
	}

	std::string getCurrentDateTime() const
	{
		time_t now = time(0);
		char buf[80];
		struct tm *timeinfo = gmtime(&now);
		strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
		return std::string(buf);
	}

public:
	HTTPResponse()
		: statusCode_(200),
		  reasonPhrase_("OK"),
		  httpVersion_("HTTP/1.1"),
		  body_("") {}

	// Builder methods with method chaining
	HTTPResponse &status(int code)
	{
		statusCode_ = code;
		reasonPhrase_ = getReasonPhrase(code);
		return *this;
	}

	HTTPResponse &status(int code, const std::string &phrase)
	{
		statusCode_ = code;
		reasonPhrase_ = phrase;
		return *this;
	}

	HTTPResponse &version(const std::string &version)
	{
		httpVersion_ = version;
		return *this;
	}

	HTTPResponse &header(const std::string &key, const std::string &value)
	{
		headers_[key] = value;
		return *this;
	}

	HTTPResponse &headers(const std::map<std::string, std::string> &headers)
	{
		for (std::map<std::string, std::string>::const_iterator it = headers.begin();
			 it != headers.end(); ++it)
			headers_[it->first] = it->second;
		return *this;
	}

	HTTPResponse &contentType(const std::string &type)
	{
		return header("Content-Type", type);
	}

	HTTPResponse &body(const std::string &content)
	{
		body_ = content;
		return *this;
	}

	HTTPResponse &json(const std::string &jsonContent)
	{
		contentType("application/json");
		return body(jsonContent);
	}

	HTTPResponse &html(const std::string &htmlContent)
	{
		contentType("text/html; charset=utf-8");
		return body(htmlContent);
	}

	HTTPResponse &text(const std::string &textContent)
	{
		contentType("text/plain; charset=utf-8");
		return body(textContent);
	}

	// Read body from file
	HTTPResponse &bodyFromFile(const std::string &filepath)
	{
		std::ifstream file(filepath.c_str(), std::ios::binary);
		if (!file.is_open())
		{
			// File not found or cannot be opened
			body_ = "";
			return *this;
		}

		// Get file size
		file.seekg(0, std::ios::end);
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		// Read entire file into string
		std::string content;
		content.reserve(size);
		content.assign(std
			::istreambuf_iterator<char>(file),
			std::istreambuf_iterator<char>()
		);

		file.close();
		body_ = content;
		contentType(getMimeType(filepath));
		return *this;
	}

	// Helper method to detect MIME type from file extension
	std::string getMimeType(const std::string &filepath) const
	{
		std::string	ext;
		size_t		pos = filepath.find_last_of('.');

		if (pos != std::string::npos)
			ext = filepath.substr(pos + 1);

		// Convert to lowercase for comparison
		for (size_t i = 0; i < ext.length(); ++i)
			ext[i] = tolower(ext[i]);

		// Common MIME types
		if (ext == "html" || ext == "htm" || ext == "py") return "text/html";
		if (ext == "css") return "text/css";
		if (ext == "js") return "application/javascript";
		if (ext == "json") return "application/json";
		if (ext == "xml") return "application/xml";
		if (ext == "pdf") return "application/pdf";
		if (ext == "zip") return "application/zip";
		if (ext == "txt") return "text/plain";
		if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
		if (ext == "png") return "image/png";
		if (ext == "gif") return "image/gif";
		if (ext == "svg") return "image/svg+xml";
		if (ext == "ico") return "image/x-icon";
		if (ext == "mp3") return "audio/mpeg";
		if (ext == "mp4") return "video/mp4";

		return "application/octet-stream"; // Default binary type
	}

	// Load file and auto-detect content type
	HTTPResponse &file(const std::string &filepath)
	{
		contentType(getMimeType(filepath));
		return bodyFromFile(filepath);
	}

	// Automatically add common headers
	HTTPResponse &autoHeaders()
	{
		if (headers_.find("Date") == headers_.end())
			header("Date", getCurrentDateTime());
		if (headers_.find("Server") == headers_.end())
			header("Server", "CustomServer/1.0");
		if (!body_.empty() && headers_.find("Content-Length") == headers_.end())
		{
			std::ostringstream oss;
			oss << body_.length();
			header("Content-Length", oss.str());
		}
		return *this;
	}

	// Build the final HTTP response string
	std::string build(bool includeBody = true) const
	{
		std::ostringstream response;

		// Status line
		response << httpVersion_ << " " << statusCode_ << " "
				 << reasonPhrase_ << "\r\n";

		// Headers
		for (std::map<std::string, std::string>::const_iterator it = headers_.begin();
			 it != headers_.end(); ++it)
			response << it->first << ": " << it->second << "\r\n";

		// Empty line separating headers from body
		response << "\r\n";

		// Body
		if (includeBody && !body_.empty())
			response << body_;

		return response.str();
	}

	// Getters
	int			getStatusCode() const { return statusCode_; }
	std::string	getBody() const { return body_; }
	std::string	getHeader(const std::string &key) const
	{
		std::map<std::string, std::string>::const_iterator it = headers_.find(key);
		return (it != headers_.end()) ? it->second : "";
	}

	void printResponse(bool includeBody = true) const
	{
		std::cout << "\n----- HTTP Response -----\n\n";
		std::cout << build(includeBody) << "\n\n============================================================\n";
	}
};

// Factory class for common responses
class ResponseFactory
{
public:
	static HTTPResponse ok()
	{
		return HTTPResponse().status(200).autoHeaders();
	}

	/**********************************************************************************************/

	static HTTPResponse badRequest_400()
	{
		return HTTPResponse()
			.status(400)
			.html("<html><body><h1>400 Bad Request</h1></body></html>")
			.autoHeaders();
	}

	static HTTPResponse unauthorized_401()
	{
		return HTTPResponse()
			.status(401)
			.html("<html><body><h1>401 Unauthorized</h1></body></html>")
			.autoHeaders();
	}

	static HTTPResponse forbidden_403()
	{
		return HTTPResponse()
			.status(403)
			.html("<html><body><h1>403 Forbidden</h1></body></html>")
			.autoHeaders();
	}

	static HTTPResponse notFound_404(std::string customMessage = "")
	{
		return HTTPResponse()
			.status(404)
			.html("<html><body><h1>404 Not Found</h1><p>" + customMessage + "</p></body></html>")
			.autoHeaders();
	}

	static HTTPResponse methodNotAllowed_405()
	{
		return HTTPResponse()
			.status(405)
			.html("<html><body><h1>405 Method Not Allowed</h1></body></html>")
			.autoHeaders();
	}

	static HTTPResponse notAcceptable_406()
	{
		return HTTPResponse()
			.status(406)
			.html("<html><body><h1>406 Not Acceptable</h1></body></html>")
			.autoHeaders();
	}

	/**********************************************************************************************/

	static HTTPResponse internalServerError_500()
	{
		return HTTPResponse()
			.status(500)
			.html("<html><body><h1>500 Internal Server Error</h1></body></html>")
			.autoHeaders();
	}

	static HTTPResponse badGateway_502()
	{
		return HTTPResponse()
			.status(502)
			.html("<html><body><h1>502 Bad Gateway</h1></body></html>")
			.autoHeaders();
	}

	static HTTPResponse serviceUnavailable_503()
	{
		return HTTPResponse()
			.status(503)
			.html("<html><body><h1>503 Service Unavailable</h1></body></html>")
			.autoHeaders();
	}

	static HTTPResponse gatewayTimeout_504()
	{
		return HTTPResponse()
			.status(504)
			.html("<html><body><h1>504 Gateway Timeout</h1></body></html>")
			.autoHeaders();
	}

	/**********************************************************************************************/

	static HTTPResponse internalError()
	{
		return HTTPResponse()
			.status(500)
			.html("<html><body><h1>500 Internal Server Error</h1></body></html>")
			.autoHeaders();
	}

	static HTTPResponse redirect(const std::string &location)
	{
		return HTTPResponse()
			.status(302)
			.header("Location", location)
			.autoHeaders();
	}
};

#endif

/*
// Example usage:
#include <iostream>

int main() {
    std::cout << "=== Example 1: Simple JSON Response ===\n";
    HttpResponse response1 = HttpResponse()
        .status(200)
        .json("{\"message\": \"Hello World\", \"status\": \"success\"}")
        .autoHeaders();
    std::cout << response1.build() << std::endl;

    std::cout << "\n=== Example 2: 404 Not Found (Factory) ===\n";
    HttpResponse response2 = ResponseFactory::notFound();
    std::cout << response2.build() << std::endl;

    std::cout << "\n=== Example 3: Complex HTML Response ===\n";
    std::string htmlContent = 
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head><title>Welcome</title></head>\n"
        "<body>\n"
        "  <h1>Welcome to Our Site</h1>\n"
        "  <p>This is a complete HTTP response example.</p>\n"
        "</body>\n"
        "</html>";
    
    HttpResponse response3 = HttpResponse()
        .status(200)
        .header("X-Custom-Header", "CustomValue")
        .header("Cache-Control", "max-age=3600")
        .header("Connection", "keep-alive")
        .html(htmlContent)
        .autoHeaders();
    std::cout << response3.build() << std::endl;

    std::cout << "\n=== Example 4: 302 Redirect ===\n";
    HttpResponse response4 = ResponseFactory::redirect("https://example.com/new-location");
    std::cout << response4.build() << std::endl;

    std::cout << "\n=== Example 5: 201 Created with JSON ===\n";
    HttpResponse response5 = HttpResponse()
        .status(201)
        .header("X-Request-ID", "abc123xyz")
        .header("Location", "/api/users/42")
        .json("{\"id\": 42, \"username\": \"john_doe\", \"created\": true}")
        .autoHeaders();
    std::cout << response5.build() << std::endl;

    std::cout << "\n=== Example 6: Plain Text Response ===\n";
    HttpResponse response6 = HttpResponse()
        .status(200)
        .text("This is a plain text response.\nIt can contain multiple lines.\nHere's line 3.")
        .autoHeaders();
    std::cout << response6.build() << std::endl;

    std::cout << "\n=== Example 7: Serve HTML File ===\n";
    // Assuming you have a file named "index.html"
    HttpResponse response7 = HttpResponse()
        .status(200)
        .file("index.html")  // Auto-detects Content-Type as text/html
        .autoHeaders();
    std::cout << response7.build() << std::endl;

    std::cout << "\n=== Example 8: Serve Image File ===\n";
    HttpResponse response8 = ResponseFactory::serveFile("logo.png");
    std::cout << response8.build() << std::endl;

    std::cout << "\n=== Example 9: Manual File Load with Custom Headers ===\n";
    HttpResponse response9 = HttpResponse()
        .status(200)
        .header("Cache-Control", "public, max-age=86400")
        .bodyFromFile("data.json")
        .contentType("application/json")
        .autoHeaders();
    std::cout << response9.build() << std::endl;

    return 0;
}
*/