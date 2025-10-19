#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include "../webserv.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Server.hpp"

// Forward declarations
class HTTPRequest;
class HTTPResponse;
class Server;
class ResponseFactory;
class CGIEnvironment
{
public:
	static std::map<std::string, std::string> buildEnvironment(
		const HTTPRequest &request,
		const Server &server,
		const std::string &scriptPath)
	{
		std::map<std::string, std::string> env;

		// CGI Required Variables
		env["REQUEST_METHOD"] = request.method;
		env["SCRIPT_NAME"] = request.uri;
		env["SCRIPT_FILENAME"] = scriptPath;
		env["QUERY_STRING"] = buildQueryString(request.queryParams);
		env["SERVER_NAME"] = server.getName();
		env["SERVER_PORT"] = server.getPort();
		env["SERVER_PROTOCOL"] = "HTTP/1.1";
		env["GATEWAY_INTERFACE"] = "CGI/1.1";

		// Request Headers as HTTP_*
		for (std::map<std::string, std::string>::const_iterator it = request.headers.begin();
			 it != request.headers.end(); ++it)
		{
			std::string headerName = "HTTP_" + it->first;
			std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::toupper);
			std::replace(headerName.begin(), headerName.end(), '-', '_');
			env[headerName] = it->second;
		}

		// Content handling
		if (request.method == "POST" || request.method == "PUT")
		{
			std::stringstream ss;
			ss << request.contentLength;
			env["CONTENT_LENGTH"] = ss.str();

			std::map<std::string, std::string>::const_iterator it = request.headers.find("Content-Type");
			if (it != request.headers.end())
				env["CONTENT_TYPE"] = it->second;
		}

		// Client information
		// env["REMOTE_ADDR"] = server.getClientAddress();
		env["REQUEST_URI"] = request.uri;
		env["PATH_INFO"] = extractPathInfo(request.uri, request.uriPath);

		return env;
	}

private:
	static std::string buildQueryString(const std::map<std::string, std::string> &params)
	{
		std::string query;
		for (std::map<std::string, std::string>::const_iterator it = params.begin();
			 it != params.end(); ++it)
		{
			if (!query.empty())
				query += "&";
			query += it->first + "=" + it->second;
		}
		return query;
	}

	static std::string extractPathInfo(const std::string &uri, const std::string &path)
	{
		if (uri.length() > path.length())
			return uri.substr(path.length());
		return "";
	}
};

class CGIExecutor
{
public:
	struct CGIResult
	{
		int exitCode;
		std::string bodyOutput; // Only the body from CGI script
		std::string errors;
		bool timedOut;

		CGIResult() : exitCode(-1), timedOut(false) {}
	};

	static CGIResult executeCGI(
		const std::string &scriptPath,
		const std::map<std::string, std::string> &environment,
		const std::string &requestBody,
		int timeoutSeconds = 30)
	{
		CGIResult result;
		int outputPipe[2]; // Pipe for capturing stdout
		int errorPipe[2];  // Pipe for capturing stderr

		// Create pipes
		if (pipe(outputPipe) == -1 || pipe(errorPipe) == -1)
		{
			result.exitCode = -1;
			result.errors = "Failed to create pipes";
			return result;
		}

		pid_t pid = fork();

		if (pid == -1)
		{
			result.exitCode = -1;
			result.errors = "Failed to fork process";
			close(outputPipe[0]);
			close(outputPipe[1]);
			close(errorPipe[0]);
			close(errorPipe[1]);
			return result;
		}

		if (pid == 0)
		{
			// Child process
			handleChildProcess(scriptPath, environment, requestBody, outputPipe, errorPipe);
			std::exit(127); // Should not reach here
		}
		else
		{
			// Parent process
			close(outputPipe[1]); // Close write end of output pipe
			close(errorPipe[1]);  // Close write end of error pipe

			result = handleParentProcess(pid, outputPipe[0], errorPipe[0], timeoutSeconds);

			close(outputPipe[0]); // Close read end
			close(errorPipe[0]);
		}

		return result;
	}

private:
	static void handleChildProcess(
		const std::string &scriptPath,
		const std::map<std::string, std::string> &environment,
		const std::string &requestBody,
		int outputPipe[2],
		int errorPipe[2])
	{
		// Redirect stdout to output pipe
		close(outputPipe[0]); // Close read end
		dup2(outputPipe[1], STDOUT_FILENO);
		close(outputPipe[1]);

		// Redirect stderr to error pipe
		close(errorPipe[0]); // Close read end
		dup2(errorPipe[1], STDERR_FILENO);
		close(errorPipe[1]);

		// Set up environment variables
		for (std::map<std::string, std::string>::const_iterator it = environment.begin();
			 it != environment.end(); ++it)
			setenv(it->first.c_str(), it->second.c_str(), 1);

		// Handle request body via stdin for POST
		if (!requestBody.empty())
		{
			int stdinPipe[2];
			if (pipe(stdinPipe) != -1)
			{
				dup2(stdinPipe[0], STDIN_FILENO);
				close(stdinPipe[0]);
				write(stdinPipe[1], requestBody.c_str(), requestBody.length());
				close(stdinPipe[1]);
			}
		}

		// Execute Python CGI script using python interpreter
		// This ensures we only run Python scripts
		const char *pythonInterpreter = "/usr/bin/python";
		const char *args[] = {pythonInterpreter, scriptPath.c_str(), NULL};
		execve(pythonInterpreter, const_cast<char *const *>(args), environ);

		// If execve fails, try python3
		pythonInterpreter = "/usr/bin/python3";
		const char *args3[] = {pythonInterpreter, scriptPath.c_str(), NULL};
		execve(pythonInterpreter, const_cast<char *const *>(args3), environ);

		// If both fail
		std::cerr << "Error executing Python CGI script: " << strerror(errno) << std::endl;
		std::exit(127);
	}

	static CGIResult handleParentProcess(pid_t pid, int outputFd, int errorFd, int timeoutSeconds)
	{
		CGIResult	result;
		int			status;
		time_t		startTime = time(NULL);

		// Set pipes to non-blocking mode
		fcntl(outputFd, F_SETFL, O_NONBLOCK);
		fcntl(errorFd, F_SETFL, O_NONBLOCK);

		// Read from pipes while waiting for child
		char buffer[4096];
		ssize_t bytesRead;

		while (true)
		{
			pid_t waitResult = waitpid(pid, &status, WNOHANG);

			// Read available output
			while ((bytesRead = read(outputFd, buffer, sizeof(buffer))) > 0)
				result.bodyOutput.append(buffer, bytesRead);

			// Read available errors
			while ((bytesRead = read(errorFd, buffer, sizeof(buffer))) > 0)
				result.errors.append(buffer, bytesRead);

			if (waitResult == pid)
			{
				// Child process finished - read any remaining data
				while ((bytesRead = read(outputFd, buffer, sizeof(buffer))) > 0)
					result.bodyOutput.append(buffer, bytesRead);
				while ((bytesRead = read(errorFd, buffer, sizeof(buffer))) > 0)
					result.errors.append(buffer, bytesRead);

				if (WIFEXITED(status))
					result.exitCode = WEXITSTATUS(status);
				else if (WIFSIGNALED(status))
				{
					result.exitCode = -1;
					result.errors += "\nProcess terminated by signal";
				}
				break;
			}
			else if (waitResult == -1)
			{
				result.exitCode = -1;
				result.errors += "\nError waiting for child process";
				break;
			}

			// Check timeout
			if (time(NULL) - startTime > timeoutSeconds)
			{
				kill(pid, SIGTERM);
				sleep(1);
				if (waitpid(pid, &status, WNOHANG) == 0)
					kill(pid, SIGKILL);
				waitpid(pid, &status, 0);
				result.timedOut = true;
				result.exitCode = -1;
				result.errors += "\nCGI script execution timeout";
				break;
			}

			usleep(50000); // Sleep 50ms before checking again
		}

		return result;
	}
};

// CGIParser removed - not needed since CGI script only returns body

class CGIHandler
{
private:
	const HTTPRequest	&request;
	const Server		&server;
	std::string			cgiDirectory;
	std::string			fileExtension;
	int					cgiTimeoutSeconds;

public:
	CGIHandler(const HTTPRequest &req, const Server &srv,
			   const std::string &cgiDir = "./cgi-bin",
			   const std::string &ext = ".py",
			   int timeout = 30)
		: request(req), server(srv), cgiDirectory(cgiDir),
		  fileExtension(ext), cgiTimeoutSeconds(timeout) {}

	bool isCGIRequest() const
	{
		return request.uriPath.find(fileExtension) != std::string::npos;
	}

	HTTPResponse executeCGI(const std::string &scriptPath)
	{
		// Verify script exists and is executable
		if (!isCGIScriptValid(scriptPath))
			return ResponseFactory::forbidden_403();

		// Only allow GET and POST methods
		if (request.method != "GET" && request.method != "POST")
			return ResponseFactory::methodNotAllowed_405();

		// Build CGI environment
		std::map<std::string, std::string> environment = CGIEnvironment::buildEnvironment(request, server, scriptPath);

		// Execute CGI script (pass body only for POST)
		std::string bodyToPass = (request.method == "POST") ? request.body : "";
		CGIExecutor::CGIResult result = CGIExecutor::executeCGI(scriptPath, environment, bodyToPass, cgiTimeoutSeconds);

		// Handle execution errors
		if (result.exitCode != 0)
		{
			if (result.timedOut)
				return ResponseFactory::gatewayTimeout_504();
			return ResponseFactory::internalServerError_500();
		}

		// Build response with body from CGI script
		// Server builds the headers, CGI only provides body
		HTTPResponse response = ResponseFactory::ok()
									.body(result.bodyOutput)
									.header("Content-Type", "text/html")
									.header("Content-Length", intToString(result.bodyOutput.length()));

		return response;
	}

private:
	bool isCGIScriptValid(const std::string &scriptPath) const
	{
		// Check if file exists
		if (access(scriptPath.c_str(), F_OK) == -1)
			return false;

		// Check if file is readable
		if (access(scriptPath.c_str(), R_OK) == -1)
			return false;

		// Check if file is executable
		if (fileExtension != ".py" && access(scriptPath.c_str(), X_OK) == -1)
			return false;

		// Optional: Check if file is in CGI directory
		// This prevents execution of arbitrary scripts

		return true;
	}

	// Helper function to convert int to string (C++98 compatible)
	static std::string intToString(size_t value)
	{
		std::stringstream ss;
		ss << value;
		return ss.str();
	}
};

// Integration with HTTPMethodHandler

#endif // CGI_HANDLER_HPP