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
	};

	/*
	to do:
		- handle cgi
		- handle 403 error
	*/
	HTTPResponse	GETHandler()
	{
		std::string	fullPathURI;
	
		if (!server.isValidUri(request.uri))
			return ResponseFactory::notFound_404();

		fullPathURI = server.getLocationValidIndex(request.uri);
		if (!server.isValidMethod(request.uri, request.method))
			return ResponseFactory::methodNotAllowed_405();
		if (ftIsFile(fullPathURI))
			return ResponseFactory::ok().bodyFromFile(fullPathURI).autoHeaders();
		else
			return ResponseFactory::forbidden_403();
	}
};

#endif

		// else if (request.method == "POST")
		// 	return POSTHandler();
		// else if (request.method == "DELETE")
		// 	return DELETEHandler();


		// else if (ftIsDirectory(fullPathURI))
		// 	return ResponseFactory::ok().bodyFromDirectory(fullPathURI).autoHeaders();