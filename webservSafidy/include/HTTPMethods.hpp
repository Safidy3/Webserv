#ifndef HTTPMETHODS_HPP
#define HTTPMETHODS_HPP

#include "../webserv.hpp"
#include "HTTPResponse.hpp"
#include "HTTPRequest.hpp"
#include "CGIHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "DataHandler.hpp"

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

	HTTPResponse	generateResponse()
	{
		std::string relativePath;

		if (!server.isValidUri(request.uriPath))
		{
			if (server.hasErrorPage(404))
			{
				std::string errorPagePath = server.getErrorPage(404);
				std::cout << "ERROR PAGE PATH : " << errorPagePath << std::endl;
				return ResponseFactory::notFound_404().bodyFromFile(errorPagePath).autoHeaders();
			}
			return ResponseFactory::notFound_404();
		}
		if (!server.isValidMethod(request.uriPath, request.method))
		{
			if (server.hasErrorPage(405))
			{
				std::string errorPagePath = server.getErrorPage(405);
				std::cout << "ERROR PAGE PATH : " << errorPagePath << std::endl;
				return ResponseFactory::methodNotAllowed_405().bodyFromFile(errorPagePath).autoHeaders();
			}
			return ResponseFactory::methodNotAllowed_405();
		}

		relativePath = server.getLocationValidIndex(request.uriPath);
		if (request.method == "GET")
			return GETHandler(relativePath);
		else if (request.method == "POST")
			return POSTHandler(relativePath);
		else if (request.method == "DELETE")
			return DELETEHandler(relativePath);
		return ResponseFactory::methodNotAllowed_405();
	}

	HTTPResponse	GETHandler(const std::string& relativePath)
	{
		// Check if CGI request
		if (cgiHandler.isCGIRequest())
			return cgiHandler.executeCGI(relativePath);
		if (ftIsFile(relativePath))
			return ResponseFactory::ok().bodyFromFile(relativePath).autoHeaders();
		else if (!ftIsFile(relativePath) && server.isLocationAutoindexOn(request.uriPath))
		{
			const std::string validIndex = server.getLocationValidIndex(request.uriPath);
			if (!validIndex.empty())
				return ResponseFactory::ok().bodyFromFile(validIndex).autoHeaders();

			std::string listPath = server.getLocationAbsolutePath(request.uriPath);
			return ResponseFactory::listDirectory(listPath, server.getRoot());
		}
		return ResponseFactory::forbidden_403();
	}

	HTTPResponse	POSTHandler(const std::string& relativePath)
	{
		HTTPResponse	response;

		// Check if CGI request
		if (cgiHandler.isCGIRequest())
		{
			response = cgiHandler.executeCGI(relativePath);
			if (response.getStatusCode() == 200)
			{
				CSVData dataHandler("data.csv");
				dataHandler.addData(
					request.getQueryParam("name"),
					request.getQueryParam("age"),
					request.getQueryParam("comment")
				);
			}
		}
		else
		{
			CSVData dataHandler("data.csv");
			dataHandler.addData(
				request.getQueryParam("name"),
				request.getQueryParam("age"),
				request.getQueryParam("comment")
			);
			response = ResponseFactory::ok().html("<html><body><h1>Data Submitted Successfully !!</h1></body></html>");
		}
		return response;
	}

	HTTPResponse	DELETEHandler(const std::string& relativePath)
	{
		HTTPResponse	response;

		// Check if CGI request
		if (cgiHandler.isCGIRequest())
		{
			response = cgiHandler.executeCGI(relativePath);
			std::cout << "RESPONSE STATUS : " << response.getStatusCode() << std::endl;
			if (response.getStatusCode() == 200)
			{
				CSVData dataHandler("data.csv");
				dataHandler.removeData(request.getQueryParam("id"));
			}
		}
		else
		{
			CSVData dataHandler("data.csv");
			dataHandler.removeData(request.getQueryParam("id"));
			response = ResponseFactory::ok().html("<html><body><h1>Data Submitted Successfully !!</h1></body></html>");
		}
		return response;
	}
};

#endif