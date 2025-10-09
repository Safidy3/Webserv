#ifndef HTTPMETHODS_HPP
#define HTTPMETHODS_HPP

#include "../webserv.hpp"
#include "HTTPResponse.hpp"
#include "HTTPRequest.hpp"
#include "Server.hpp"
#include "Client.hpp"

class HTTPMethodHandler
{
private:
	const HTTPRequest&	request;
	const Server&		server;

public:
	HTTPMethodHandler(const HTTPRequest &req, const Server &srv) : request(req), server(srv) {}
	~HTTPMethodHandler() {}

	HTTPResponse	generateResponse()
	{
		if (request.method == "GET")
			return GETHandler();
		else
			return ResponseFactory::methodNotAllowed_405();
		// else if (request.method == "POST")
		// 	return POSTHandler();
		// else if (request.method == "DELETE")
		// 	return DELETEHandler();
	};

	HTTPResponse	GETHandler()
	{
		std::string	relativePath;
		if (!server.isValidUri(request.uri))
			return ResponseFactory::notFound_404();

		relativePath = server.getLocationValidIndex(request.uri);
		if (!server.isValidMethod(request.uri, request.method))
			return ResponseFactory::methodNotAllowed_405();
		if (ftIsFile(relativePath))
			return ResponseFactory::ok().bodyFromFile(relativePath).autoHeaders();
		// else if (ftIsDirectory(relativePath))
		// 	return ResponseFactory::ok().bodyFromDirectory(relativePath).autoHeaders();
		else
			return ResponseFactory::forbidden_403();
	}
};

#endif
