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

	HTTPResponse	serveStaticFile(const std::string &path)
	{
		return ResponseFactory::ok().bodyFromFile(path).autoHeaders();
	}

	// HTTPResponse	serveDirectory(const std::string &path)
	// {
	// 	return ResponseFactory::ok().bodyFromDirectory(path).autoHeaders();
	// }

public:
	HTTPMethodHandler(const HTTPRequest &req, const Server &srv) : request(req), server(srv) {}
	~HTTPMethodHandler() {}
	
	HTTPResponse	generateResponse()
	{
		if (request.getMethod() == "GET")
			return GETHandler();
		else
			return ResponseFactory::methodNotAllowed_405();
		// else if (request.getMethod() == "POST")
		// 	return POSTHandler();
		// else if (request.getMethod() == "DELETE")
		// 	return DELETEHandler();
	};

	HTTPResponse	GETHandler()
	{
		std::string	relativePath = server.getLocationValidIndex(request.getUri());

		// if (!server.isValidUri(request.getUri()))
		// 	return ResponseFactory::notFound_404();
		if (!server.isValidMethod(request.getUri(), request.getMethod()))
			return ResponseFactory::methodNotAllowed_405();

		if (ftIsFile(relativePath))
			return serveStaticFile(relativePath);
		// else if (ftIsDirectory(relativePath))
		// 	return serveDirectory(relativePath);
		else
			return ResponseFactory::forbidden_403();
	}
};

/*===============================================*/

/*
class HTTPMethodFactory
{
private:
	std::map<std::string, AHTTPMethodHandler *> _handlers;

public:
	HTTPMethodFactory()
	{
		_handlers["GET"] = new GETHandler();
		// _handlers["POST"] = new POSTHandler();
		// _handlers["DELETE"] = new DELETEHandler();
	}

	// Clean up all handlers
	~HTTPMethodFactory()
	{
		for (std::map<std::string, AHTTPMethodHandler *>::iterator it = _handlers.begin(); it != _handlers.end(); ++it)
			delete it->second;
	}

	AHTTPMethodHandler *getHandler(const std::string &method)
	{
		if (_handlers.find(method) != _handlers.end())
			return _handlers[method];
		return NULL;
	}
};
*/

#endif


/*
// In your Server class:
class Server {
private:
	HTTPMethodFactory _methodFactory;
	
public:
	void handleRequest(int clientFd) {
		Request request = _requestParser.parse(clientFd);
		
		AHTTPMethodHandler* handler = _methodFactory.getHandler(request.getMethod());
		
		Response response;
		if (handler) {
			response = handler->execute(request);
		} else {
			response = _responseBuilder.buildError(405, "Method Not Allowed");
		}
		
		_responseBuilder.send(clientFd, response);
	}
};
*/