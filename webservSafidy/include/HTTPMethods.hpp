#ifndef HTTPMETHODS_HPP
#define HTTPMETHODS_HPP

#include "../webserv.hpp"
#include "HTTPResponse.hpp"
#include "HTTPRequest.hpp"
#include "CGIHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"

class HTTPMethodHandler
{
private:
	const HTTPRequest	&request;
	const Server		&server;
	CGIHandler			cgiHandler;

public:
	HTTPMethodHandler(const HTTPRequest &req, const Server &srv)
		: request(req), server(srv), cgiHandler(req, srv, req.uriPath){}

	~HTTPMethodHandler() {}

	HTTPResponse generateResponse()
	{
		if (request.method == "GET")
			return GETHandler();
		else if (request.method == "POST")
			return POSTHandler();
		else
			return ResponseFactory::methodNotAllowed_405();
	}

	HTTPResponse GETHandler()
	{
		std::string relativePath;

		if (!server.isValidUri(request.uriPath))
			return ResponseFactory::notFound_404();

		relativePath = server.getLocationValidIndex(request.uriPath);
		if (!server.isValidMethod(request.uriPath, request.method))
			return ResponseFactory::methodNotAllowed_405();

		// Check if CGI request
		if (cgiHandler.isCGIRequest())
			return cgiHandler.executeCGI(relativePath);

		if (ftIsFile(relativePath))
			return ResponseFactory::ok().bodyFromFile(relativePath).autoHeaders();
		else
			return ResponseFactory::forbidden_403();
	}

	HTTPResponse POSTHandler()
	{
		std::string relativePath;

		if (!server.isValidUri(request.uriPath))
			return ResponseFactory::notFound_404();

		relativePath = server.getLocationValidIndex(request.uriPath);
		if (!server.isValidMethod(request.uriPath, request.method))
			return ResponseFactory::methodNotAllowed_405();

		// Check if CGI request
		if (cgiHandler.isCGIRequest())
			return cgiHandler.executeCGI(relativePath);
		return ResponseFactory::methodNotAllowed_405();
	}
};


#endif