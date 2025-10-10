#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "../webserv.hpp"

class HTTPRequest
{
public:
	HTTPRequest() : contentLength(0) {}

	std::string							method;
	std::string							uri;
	std::string							path;
	std::string							version;
	std::map<std::string, std::string>	headers;
	std::map<std::string, std::string>	queryParams;
	std::string							body;
	size_t								contentLength;

	std::string getHeader(const std::string& name) const
	{
		std::map<std::string, std::string>::const_iterator it = 
			headers.find(toLower(name));
		if (it != headers.end())
			return it->second;
		return "";
	}

	std::string getQueryParam(const std::string& name) const
	{
		std::map<std::string, std::string>::const_iterator it = 
			queryParams.find(name);
		if (it != queryParams.end())
			return it->second;
		return "";
	}

	void	printRequest() const
	{
		std::cout << "----- HTTP Request -----\n";
		std::cout << "Request:\n";
		std::cout << "\t" << method << " " << uri << " " << version << "\n";

		if (!queryParams.empty())
			std::cout << "Query Parameters:\n";
		for (std::map<std::string, std::string>::const_iterator it = queryParams.begin();
			 it != queryParams.end(); ++it)
			std::cout << "\t" << it->first << ": " << it->second << "\n";

		std::cout << "Headers:\n";
		for (std::map<std::string, std::string>::const_iterator it = headers.begin();
			 it != headers.end(); ++it)
			std::cout << "\t" << it->first << ": " << it->second << "\n";

		std::cout << "Body:\n";
		if (!body.empty())
			std::cout  << "\t" << "\n" << body << "\n";

		std::cout << "------------------------\n";
	}

private:
	static std::string toLower(const std::string& str)
	{
		std::string result = str;
		for (size_t i = 0; i < result.length(); ++i)
			result[i] = std::tolower(result[i]);
		return result;
	}
};

class HTTPRequestParser
{
public:
	enum ParserState
	{
		STATE_REQUEST_LINE,
		STATE_HEADERS,
		STATE_BODY,
		STATE_COMPLETE,
		STATE_ERROR
	};

	HTTPRequestParser() : state(STATE_REQUEST_LINE), bodyBytesRead(0) {}

	ParserState parse(const std::string& data)
	{
		buffer += data;
		while (state != STATE_COMPLETE && state != STATE_ERROR)
		{
			size_t crlfPos = buffer.find("\r\n");
			
			if (state == STATE_REQUEST_LINE)
			{
				if (crlfPos == std::string::npos)
					break;
				if (!parseRequestLine(buffer.substr(0, crlfPos)))
				{
					state = STATE_ERROR;
					return state;
				}
				buffer = buffer.substr(crlfPos + 2);
				state = STATE_HEADERS;
			}
			else if (state == STATE_HEADERS)
			{
				if (crlfPos == std::string::npos)
					break;
				std::string headerLine = buffer.substr(0, crlfPos);
				if (headerLine.empty())
				{
					buffer = buffer.substr(crlfPos + 2);
					if (request.contentLength > 0)
						state = STATE_BODY;
					else
						state = STATE_COMPLETE;
				}
				else
				{
					if (!parseHeader(headerLine))
					{
						state = STATE_ERROR;
						return state;
					}
					buffer = buffer.substr(crlfPos + 2);
				}
			}
			else if (state == STATE_BODY)
			{
				if (bodyBytesRead < request.contentLength)
				{
					size_t bytesNeeded = request.contentLength - bodyBytesRead;
					size_t bytesAvailable = buffer.length();
					size_t bytesToRead = (bytesAvailable < bytesNeeded) ? 
										bytesAvailable : bytesNeeded;
					if (bytesToRead > 0)
					{
						request.body += buffer.substr(0, bytesToRead);
						buffer = buffer.substr(bytesToRead);
						bodyBytesRead += bytesToRead;
					}
				}
				if (bodyBytesRead >= request.contentLength)
					state = STATE_COMPLETE;
				else
					break;
			}
		}
		return state;
	}

	const HTTPRequest& getRequest() const
	{
		return request;
	}

	std::string getErrorMessage() const
	{
		return errorMessage;
	}

	ParserState getState() const
	{
		return state;
	}

	void reset()
	{
		state = STATE_REQUEST_LINE;
		buffer.clear();
		bodyBytesRead = 0;
		request = HTTPRequest();
		errorMessage.clear();
	}

private:
	ParserState state;
	HTTPRequest request;
	std::string	errorMessage;
	std::string buffer;
	size_t		bodyBytesRead;

	bool parseRequestLine(const std::string& line)
	{
		std::istringstream iss(line);
		std::string method, uri, version;
		
		if (!(iss >> method >> uri >> version))
		{
			errorMessage = "Invalid request line";
			return false;
		}
		if (method != "GET" && method != "POST" && method != "PUT" &&
			method != "DELETE" && method != "HEAD" && method != "OPTIONS" &&
			method != "PATCH" && method != "TRACE" && method != "CONNECT")
		{
			errorMessage = "Invalid HTTP method";
			return false;
		}
		if (uri.empty() || uri[0] != '/')
		{
			errorMessage = "Invalid URI";
			return false;
		}
		if (version.substr(0, 5) != "HTTP/")
		{
			errorMessage = "Invalid HTTP version";
			return false;
		}

		request.method = method;
		request.uri = uri;
		request.version = version;
		parseQueryParams(uri);
		return true;
	}

	void parseQueryParams(const std::string& uri)
	{
		size_t queryStart = uri.find('?');
		
		if (queryStart == std::string::npos)
		{
			request.path = uri;
			return;
		}
		
		request.path = uri.substr(0, queryStart);
		std::string queryString = uri.substr(queryStart + 1);
		
		size_t pos = 0;
		while (pos < queryString.length())
		{
			size_t ampPos = queryString.find('&', pos);
			if (ampPos == std::string::npos)
				ampPos = queryString.length();
			
			std::string param = queryString.substr(pos, ampPos - pos);
			
			size_t eqPos = param.find('=');
			if (eqPos != std::string::npos)
			{
				std::string key = param.substr(0, eqPos);
				std::string value = param.substr(eqPos + 1);
				
				key = urlDecode(key);
				value = urlDecode(value);
				
				request.queryParams[key] = value;
			}
			else if (!param.empty())
			{
				request.queryParams[urlDecode(param)] = "";
			}
			
			pos = ampPos + 1;
		}
	}

	static std::string urlDecode(const std::string& encoded)
	{
		std::string decoded;
		for (size_t i = 0; i < encoded.length(); ++i)
		{
			if (encoded[i] == '%' && i + 2 < encoded.length())
			{
				std::string hex = encoded.substr(i + 1, 2);
				char c = static_cast<char>(strtol(hex.c_str(), NULL, 16));
				decoded += c;
				i += 2;
			}
			else if (encoded[i] == '+')
				decoded += ' ';
			else
				decoded += encoded[i];
		}
		return decoded;
	}

	bool parseHeader(const std::string& line)
	{
		size_t colonPos = line.find(':');
		
		if (colonPos == std::string::npos)
		{
			errorMessage = "Invalid header format";
			return false;
		}
		
		std::string name = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);
		
		// Trim leading/trailing whitespace from value
		size_t start = value.find_first_not_of(" \t");
		size_t end = value.find_last_not_of(" \t");
		
		if (start != std::string::npos)
			value = value.substr(start, end - start + 1);
		else
			value.clear();
		
		// Convert header name to lowercase for case-insensitive lookup
		std::string lowerName = name;
		for (size_t i = 0; i < lowerName.length(); ++i)
			lowerName[i] = std::tolower(lowerName[i]);
		
		request.headers[lowerName] = value;
		
		// Extract Content-Length if present
		if (lowerName == "content-length")
		{
			std::istringstream iss(value);
			iss >> request.contentLength;
		}
		
		return true;
	}
};
#endif
