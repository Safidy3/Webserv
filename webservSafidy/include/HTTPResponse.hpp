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

// Directory listing builder class
class DirectoryListing
{
private:
	std::string					path_;
	std::string					title_;
	std::vector<DirectoryEntry> entries_;
	bool						showParent_;
	std::string					serverRoot_;

	std::string formatSize(long bytes) const
	{
		if (bytes < 1024)
		{
			std::ostringstream oss;
			oss << bytes << " B";
			return oss.str();
		}
		else if (bytes < 1024 * 1024)
		{
			std::ostringstream oss;
			oss << (bytes / 1024) << " KB";
			return oss.str();
		}
		else if (bytes < 1024 * 1024 * 1024)
		{
			std::ostringstream oss;
			oss << (bytes / (1024 * 1024)) << " MB";
			return oss.str();
		}
		else
		{
			std::ostringstream oss;
			oss << (bytes / (1024 * 1024 * 1024)) << " GB";
			return oss.str();
		}
	}

	std::string formatTime(time_t timestamp) const
	{
		char buf[80];
		struct tm* timeinfo = localtime(&timestamp);
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", timeinfo);
		return std::string(buf);
	}

	std::string escapeHtml(const std::string& text) const
	{
		std::string result;
		for (size_t i = 0; i < text.length(); ++i)
		{
			switch (text[i])
			{
				case '&': result += "&amp;"; break;
				case '<': result += "&lt;"; break;
				case '>': result += "&gt;"; break;
				case '"': result += "&quot;"; break;
				case '\'': result += "&#39;"; break;
				default: result += text[i];
			}
		}
		return result;
	}

	bool scanDirectory(const std::string& dirPath)
	{
		entries_.clear();

		DIR* dir = opendir(dirPath.c_str());
		if (!dir)
			return false;

		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL)
		{
			std::string name = entry->d_name;
			if (name != "." && name != "..")
			{
				std::string fullPath = dirPath + "/" + name;
				struct stat statbuf;
				
				if (stat(fullPath.c_str(), &statbuf) == 0)
				{
					bool isDir = S_ISDIR(statbuf.st_mode);
					long size = isDir ? 0 : statbuf.st_size;
					std::string modTime = formatTime(statbuf.st_mtime);
					entries_.push_back(DirectoryEntry(name, isDir, size, modTime));
				}
			}
		}
		closedir(dir);
		std::sort(entries_.begin(), entries_.end(), entrySorter);
		return true;
	}

	static bool entrySorter(const DirectoryEntry& a, const DirectoryEntry& b)
	{
		if (a.isDirectory != b.isDirectory)
			return a.isDirectory; // Directories first
		return a.name < b.name; // Alphabetical
	}

public:
	DirectoryListing(const std::string& path, const std::string& serverRoot = "")
		: path_(path), title_("Index of " + path), showParent_(true), serverRoot_(serverRoot) {}

	DirectoryListing& title(const std::string& t)
	{
		title_ = t;
		return *this;
	}

	DirectoryListing& showParentDirectory(bool show)
	{
		showParent_ = show;
		return *this;
	}

	std::string buildHtml()
	{
		if (!scanDirectory(path_))
			return "<html><body><h1>Error: Cannot read directory</h1></body></html>";

		std::ostringstream	html;
		html << "<html>\n"
			<< "<head><style>" 
			<< "	body {"
			<< "	    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Arial, sans-serif;"
			<< "	    max-width: 1200px;"
			<< "	    margin: 0 auto;"
			<< "	    padding: 20px;"
			<< "	    background: #f5f5f5;"
			<< "	}"
			<< "	h1 {"
			<< "	    color: #333;"
			<< "	    border-bottom: 2px solid #0066cc;"
			<< "	    padding-bottom: 10px;"
			<< "	}"
			<< "	table {"
			<< "	    width: 100%;"
			<< "	    background: white;"
			<< "	    border-collapse: collapse;"
			<< "	    box-shadow: 0 2px 4px rgba(0,0,0,0.1);"
			<< "	}"
			<< "	th {"
			<< "	    background: #0066cc;"
			<< "	    color: white;"
			<< "	    padding: 12px;"
			<< "	    text-align: left;"
			<< "	    font-weight: 600;"
			<< "	}"
			<< "	td {"
			<< "	    padding: 10px 12px;"
			<< "	    border-bottom: 1px solid #eee;"
			<< "	}"
			<< "	tr:hover {"
			<< "	    background: #f9f9f9;"
			<< "	}"
			<< "	a {"
			<< "	    color: #0066cc;"
			<< "	    text-decoration: none;"
			<< "	}"
			<< "	a:hover {"
			<< "	    text-decoration: underline;"
			<< "	}"
			<< "	.icon {"
			<< "	    display: inline-block;"
			<< "	    width: 20px;"
			<< "	    text-align: center;"
			<< "	    margin-right: 8px;"
			<< "	}"
			<< "	.dir { color: #f39c12; }"
			<< "	.file { color: #3498db; }"
			<< "	.size { text-align: right; }"
			<< "	.date { color: #777; }"
			<< "</style></head>\n"
			<< "<body><h1>" << escapeHtml(title_) << "</h1>\n"
			<< "<table><thead><tr>\n"
			<< "	<th>Name</th>\n"
			<< "	<th class=\"size\">Size</th>\n"
			<< "	<th>Modified</th>\n"
			<< "</tr></thead><tbody>\n";

		// Parent directory link
		if (showParent_)
		{
			html << "<tr>\n"
				<< "    <td><span class=\"icon dir\">📁</span><a href=\"../\">Parent Directory</a></td>\n"
				<< "    <td class=\"size\">-</td>\n"
				<< "    <td class=\"date\">-</td>\n"
				<< "</tr>\n";
		}

		// Directory entries
		for (size_t i = 0; i < entries_.size(); ++i)
		{
			const DirectoryEntry&	entry = entries_[i];
			std::string				icon = entry.isDirectory ? "📁" : "📄";
			std::string				iconClass = entry.isDirectory ? "dir" : "file";

			std::string fullPath = path_ + "/" + entry.name;
			std::string href = fullPath.substr(serverRoot_.length()); // Remove server root and ensure it starts with /

			if (!href.empty() && href[0] != '/') // href starts with /
				href = "/" + href;

			// std::cout << "\n****** SERVER ROOT : " << serverRoot_ << std::endl;
			// std::cout << "****** PATH        : " << path_ << std::endl;
			// std::cout << "****** ENTRY       : " << entry.name << std::endl;
			// std::cout << "****** HREF        : " << href << std::endl << std::endl;

			html << "<tr><td><span class=\"icon " << iconClass << "\">" << icon << "</span>"
				<< "<a href=\"" << href << "\">" << escapeHtml(entry.name) << "</a></td>\n"
				<< "<td class=\"size\">" << (entry.isDirectory ? "-" : formatSize(entry.size)) << "</td>\n"
				<< "<td class=\"date\">" << escapeHtml(entry.modifiedTime) << "</td></tr>\n";
		}
		html << "</tbody></table></body></html>";

		return html.str();
	}

	HTTPResponse buildResponse()
	{
		return HTTPResponse()
			.status(200)
			.html(buildHtml())
			.autoHeaders();
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

	static HTTPResponse listDirectory(const std::string& location, const std::string& serverRoot)
	{
		DirectoryListing listing(location, serverRoot);
		return listing.buildResponse();
	}
};

#endif
