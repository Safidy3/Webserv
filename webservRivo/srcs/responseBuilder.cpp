/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   responseBuilder.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 17:17:28 by rhanitra          #+#    #+#             */
/*   Updated: 2025/11/01 10:31:58 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpResponse.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include "../include/utils.hpp"
#include "../include/handleErrors.hpp"

// Forward declarations for utils functions (ensures visibility)
std::string parseCGIStatusFromHeaders(const std::string &headers);
std::string generateAutoindexHTML(const std::string &dirPath, const std::string &uri);

// ...existing code...

HttpResponseBuilder::HttpResponseBuilder(const MimeTypes &types)
        : _mimeTypes(types) {}

HttpResponseBuilder::~HttpResponseBuilder() {}

std::string HttpResponseBuilder::getMimeType(const std::string &path)
{
    std::string::size_type dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos)
        return ("application/octet-stream");

    std::string ext = path.substr(dotPos + 1);
    // normalize extension to lowercase to support files with uppercase extensions
    for (size_t i = 0; i < ext.size(); ++i) ext[i] = tolower(ext[i]);
    std::map<std::string, std::string>::const_iterator it = _mimeTypes.types.find(ext);
    if (it != _mimeTypes.types.end())
        return (it->second);
    return ("application/octet-stream");
}

std::string HttpResponseBuilder::buildResponse(
    const HttpRequest &req, 
    const ServerConfig &serverConf, 
    const LocationConfig &locationConf)
{
    std::string body;
    std::string statusLine = "HTTP/1.1 200 OK\r\n";
    std::string headers;

    try
    {
        // =====================================================
        // 🔹 GESTION DE LA MÉTHODE DELETE
        // =====================================================
        if (req.method == "DELETE") {
            std::string filePath = resolveFilePath(req, serverConf, locationConf);

            std::cerr << "[DELETE] filePath='" << filePath << "'\n";

            struct stat st;
            if (stat(filePath.c_str(), &st) != 0)
                return HandleErrors::generateErrorResponse(404, serverConf, &locationConf);

            if (S_ISDIR(st.st_mode))
                return HandleErrors::generateErrorResponse(403, serverConf, &locationConf);

            if (unlink(filePath.c_str()) != 0)
                return HandleErrors::generateErrorResponse(500, serverConf, &locationConf);

            // ✅ Suppression réussie → 204 No Content
            std::string response;
            response  = "HTTP/1.1 204 No Content\r\n";
            response += "Content-Length: 0\r\n";
            response += "Connection: close\r\n\r\n";
            return response;
        }

        // =====================================================
        // 🔹 GESTION CGI
        // =====================================================
        std::string cgiScript;
        if (isCgiRequest(req, locationConf, cgiScript))
        {
            HandleCGI cgi(req, serverConf, locationConf);
            cgi.buildEnv();
            std::string cgiOutput = cgi.execute();

            size_t pos = cgiOutput.find("\r\n\r\n");
            if (pos != std::string::npos) {
                headers = cgiOutput.substr(0, pos);
                body = cgiOutput.substr(pos + 4);

                // Vérifie si un Content-Length est déjà défini
                if (headers.find("Content-Length") == std::string::npos) {
                    headers += "\r\nContent-Length: " + ftToString(body.size());
                } else {
                    // ⚠️ recalculer le Content-Length du vrai body
                    size_t clPos = headers.find("Content-Length:");
                    if (clPos != std::string::npos) {
                        size_t end = headers.find("\r\n", clPos);
                        headers.replace(clPos, end - clPos, "Content-Length: " + ftToString(body.size()));
                    }
                }

                // parse CGI Status header if present
                std::string parsed = parseCGIStatusFromHeaders(headers);
                if (!parsed.empty())
                    statusLine = parsed;
            } else {
                // Pas d'en-têtes CGI explicites
                headers = "Content-Type: text/html; charset=UTF-8\r\n";
                body = cgiOutput;
                headers += "Content-Length: " + ftToString(body.size()) + "\r\n";
            }
        }
        else
        {
            // =====================================================
            // 🔹 GESTION FICHIERS STATIQUES
            // =====================================================
            std::string root = !locationConf.root.empty() ? locationConf.root : serverConf.root;

            std::string relativePath;
            if (!locationConf.path.empty() && req.uri.find(locationConf.path) == 0)
                relativePath = req.uri.substr(locationConf.path.size());
            else if (!req.uri.empty() && req.uri[0] == '/')
                relativePath = req.uri.substr(1);
            else
                relativePath = req.uri;

            if (relativePath.empty() || relativePath == "/") {
                if (!locationConf.indexFiles.empty())
                    relativePath = locationConf.indexFiles[0];
                else if (!serverConf.indexFiles.empty())
                    relativePath = serverConf.indexFiles[0];
                else
                    relativePath = "index.html";
            }

            std::string filePath = root + "/" + relativePath;
            std::cerr << "[DEBUG] root='" << root << "' relativePath='" 
                      << relativePath << "' uri='" << req.uri 
                      << "' filePath='" << filePath << "'\n";

            struct stat st;
            if (stat(filePath.c_str(), &st) != 0) {
                std::string uriPath = req.uri;
                if (!uriPath.empty() && uriPath[0] == '/') uriPath = uriPath.substr(1);
                std::string altPath = root + "/" + uriPath;
                std::cerr << "responseBuilder: trying altPath='" << altPath << "'\n";
                if (stat(altPath.c_str(), &st) == 0)
                    filePath = altPath;
            }

            // Dossier → autoindex ou index.html
            if (stat(filePath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
                if (locationConf.autoindex) {
                    std::string ai = generateAutoindexHTML(filePath, req.uri);
                    if (!ai.empty()) {
                        body = ai;
                        headers = "Content-Type: text/html; charset=UTF-8\r\n";
                        headers += "Content-Length: " + ftToString(body.size()) + "\r\n";
                    } else {
                        statusLine = "HTTP/1.1 403 Forbidden\r\n";
                        body = "<h1>403 Forbidden</h1>";
                        headers = "Content-Type: text/html; charset=UTF-8\r\n";
                        headers += "Content-Length: " + ftToString(body.size()) + "\r\n";
                    }
                } else {
                    std::string idxPath;
                    if (!locationConf.indexFiles.empty()) idxPath = root + "/" + locationConf.indexFiles[0];
                    else if (!serverConf.indexFiles.empty()) idxPath = root + "/" + serverConf.indexFiles[0];
                    else idxPath = root + "/index.html";

                    body = ftReadFile(idxPath);
                    if (body.empty()) {
                        statusLine = "HTTP/1.1 403 Forbidden\r\n";
                        body = "<h1>403 Forbidden</h1>";
                        headers = "Content-Type: text/html; charset=UTF-8\r\n";
                        headers += "Content-Length: " + ftToString(body.size()) + "\r\n";
                    } else {
                        std::string contentType = getMimeType(idxPath);
                        if (contentType.find("text/") == 0 || contentType == "application/json")
                            contentType += "; charset=UTF-8";
                        headers = "Content-Type: " + contentType + "\r\n";
                        headers += "Content-Length: " + ftToString(body.size()) + "\r\n";
                    }
                }
            } 
            else {
                body = ftReadFile(filePath);
                std::string contentType = getMimeType(filePath);

                // ✅ Ajout charset uniquement pour les types textuels
                if (contentType.find("text/") == 0 || contentType == "application/json")
                    contentType += "; charset=UTF-8";

                headers = "Content-Type: " + contentType + "\r\n";
                headers += "Content-Length: " + ftToString(body.size()) + "\r\n";
            }
        }
    }
    catch (...)
    {
        statusLine = "HTTP/1.1 404 Not Found\r\n";
        body = "<h1>404 Not Found</h1>";
        headers = "Content-Type: text/html; charset=UTF-8\r\n";
        headers += "Content-Length: " + ftToString(body.size()) + "\r\n";
    }

    std::ostringstream response;

    // ✅ S'assurer que les headers se terminent bien par \r\n
    if (headers.size() < 2 || headers.substr(headers.size() - 2) != "\r\n")
        headers += "\r\n";

    response << statusLine << headers << "\r\n";
    response.write(body.data(), body.size());
    return response.str();

}



/*std::string HttpResponseBuilder::buildResponse(
    const HttpRequest &req, 
    const ServerConfig &serverConf, 
    const LocationConfig &locationConf)
{
    std::string body;
    std::string statusLine = "HTTP/1.1 200 OK\r\n";
    std::string headers;

    try
    {
        // =====================================================
        // 🔹 GESTION DE LA MÉTHODE DELETE
        // =====================================================
        if (req.method == "DELETE") {
            std::string filePath = resolveFilePath(req, serverConf, locationConf);

            std::cerr << "[DELETE] filePath='" << filePath << "'\n";

            struct stat st;
            if (stat(filePath.c_str(), &st) != 0)
                return HandleErrors::generateErrorResponse(404, serverConf, &locationConf);

            if (S_ISDIR(st.st_mode))
                return HandleErrors::generateErrorResponse(403, serverConf, &locationConf);

            if (unlink(filePath.c_str()) != 0)
                return HandleErrors::generateErrorResponse(500, serverConf, &locationConf);

            std::string response;
            response  = "HTTP/1.1 204 No Content\r\n";
            response += "Content-Length: 0\r\n";
            response += "Connection: close\r\n\r\n";
            return response;
        }

        // 🔹 Vérifier si la requête correspond à un CGI
        std::string cgiScript;
        if (isCgiRequest(req, locationConf, cgiScript))
        {
            HandleCGI cgi(req, serverConf, locationConf);
            cgi.buildEnv();
            std::string cgiOutput = cgi.execute();

            size_t pos = cgiOutput.find("\r\n\r\n");
            if (pos != std::string::npos) {
                headers = cgiOutput.substr(0, pos);
                body = cgiOutput.substr(pos + 4);
                std::string parsed = parseCGIStatusFromHeaders(headers);
                if (!parsed.empty())
                    statusLine = parsed;
            } else {
                headers = "Content-Type: text/html\r\n";
                body = cgiOutput;
            }

            if (headers.find("Content-Length") == std::string::npos)
                headers += "\r\nContent-Length: " + ftToString(body.size());
        }
        else
        {
            std::string root = !locationConf.root.empty() ? locationConf.root : serverConf.root;
            std::string relativePath;

            if (!locationConf.path.empty() && req.uri.find(locationConf.path) == 0)
                relativePath = req.uri.substr(locationConf.path.size());
            else if (!req.uri.empty() && req.uri[0] == '/')
                relativePath = req.uri.substr(1);
            else
                relativePath = req.uri;

            if (relativePath.empty() || relativePath == "/") {
                if (!locationConf.indexFiles.empty())
                    relativePath = locationConf.indexFiles[0];
                else if (!serverConf.indexFiles.empty())
                    relativePath = serverConf.indexFiles[0];
                else
                    relativePath = "index.html";
            }

            std::string filePath = root + "/" + relativePath;
            struct stat st;
            std::cerr << "[DEBUG] root='" << root << "' relativePath='" << relativePath 
                      << "' uri='" << req.uri << "' filePath='" << filePath << "'\n";

            if (stat(filePath.c_str(), &st) != 0) {
                std::string uriPath = req.uri;
                if (!uriPath.empty() && uriPath[0] == '/') uriPath = uriPath.substr(1);
                std::string altPath = root + "/" + uriPath;
                std::cerr << "responseBuilder: trying altPath='" << altPath << "'\n";
                if (stat(altPath.c_str(), &st) == 0)
                    filePath = altPath;
            }

            if (stat(filePath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
                if (locationConf.autoindex) {
                    std::string ai = generateAutoindexHTML(filePath, req.uri);
                    if (!ai.empty()) {
                        body = ai;
                        headers = "Content-Type: text/html\r\n";
                        headers += "Content-Length: " + ftToString(body.size()) + "\r\n";
                    } else {
                        statusLine = "HTTP/1.1 403 Forbidden\r\n";
                        body = "<h1>403 Forbidden</h1>";
                        headers = "Content-Length: " + ftToString(body.size()) + "\r\n";
                        headers += "Content-Type: text/html\r\n";
                    }
                } else {
                    std::string idxPath;
                    if (!locationConf.indexFiles.empty()) idxPath = root + "/" + locationConf.indexFiles[0];
                    else if (!serverConf.indexFiles.empty()) idxPath = root + "/" + serverConf.indexFiles[0];
                    else idxPath = root + "/index.html";

                    body = ftReadFile(idxPath);
                    std::string contentType = getMimeType(idxPath);
                    headers = "Content-Type: " + contentType + "\r\n";
                    headers += "Content-Length: " + ftToString(body.size()) + "\r\n";
                }
            } else {
                body = ftReadFile(filePath);
                std::string contentType = getMimeType(filePath);
                headers = "Content-Type: " + contentType + "\r\n";
                headers += "Content-Length: " + ftToString(body.size()) + "\r\n";
            }
        }
    }
    catch (...)
    {
        statusLine = "HTTP/1.1 404 Not Found\r\n";
        body = "<h1>404 Not Found</h1>";
        headers = "Content-Length: " + ftToString(body.size()) + "\r\n";
        headers += "Content-Type: text/html\r\n";
    }

    // ✅ Construction sûre : HEADERS + DOUBLE CRLF + BODY (sans concaténer binaire)
    std::ostringstream response;
    response << statusLine << headers << "\r\n";
    response.write(body.data(), body.size());
    return response.str();

}*/


/*std::string HttpResponseBuilder::buildResponse(
    const HttpRequest &req, 
    const ServerConfig &serverConf, 
    const LocationConfig &locationConf)
{
    std::string body;
    std::string statusLine = "HTTP/1.1 200 OK\r\n";
    std::string headers;

    try
    {
        // =====================================================
        // 🔹 GESTION DE LA MÉTHODE DELETE
        // =====================================================
        if (req.method == "DELETE") {
            std::string filePath = resolveFilePath(req, serverConf, locationConf);

            std::cerr << "[DELETE] filePath='" << filePath << "'\n";

            struct stat st;
            if (stat(filePath.c_str(), &st) != 0) {
                // 🔸 Fichier non trouvé
                return HandleErrors::generateErrorResponse(404, serverConf, &locationConf);
            }

            if (S_ISDIR(st.st_mode)) {
                // 🔸 Tentative de suppression d’un dossier
                return HandleErrors::generateErrorResponse(403, serverConf, &locationConf);
            }

            if (unlink(filePath.c_str()) != 0) {
                // 🔸 Erreur système (permissions, etc.)
                return HandleErrors::generateErrorResponse(500, serverConf, &locationConf);
            }

            // ✅ Suppression réussie → 204 No Content
            std::string response;
            response  = "HTTP/1.1 204 No Content\r\n";
            response += "Content-Length: 0\r\n";
            response += "Connection: close\r\n\r\n";
            return response;
        }


        // 🔹 Vérifier si la requête correspond à un CGI
        std::string cgiScript;
        if (isCgiRequest(req, locationConf, cgiScript))
        {
            HandleCGI cgi(req, serverConf, locationConf);
            cgi.buildEnv();
            std::string cgiOutput = cgi.execute();

            // Séparer headers et body envoyés par le CGI
            size_t pos = cgiOutput.find("\r\n\r\n");
            if (pos != std::string::npos) {
                headers = cgiOutput.substr(0, pos);
                body = cgiOutput.substr(pos + 4);

                    // parse CGI Status header if present
                    {
                        std::string parsed = parseCGIStatusFromHeaders(headers);
                        if (!parsed.empty()) statusLine = parsed;
                    }
            } else {
                headers = "Content-Type: text/html; charset=UTF-8\r\n";
                body = cgiOutput;
            }

            if (headers.find("Content-Length") == std::string::npos)
                headers += "\r\nContent-Length: " + ftToString(body.size());
        }
        else
        {
            // 🔹 Fichier statique : gérer root location + index
            std::string root = !locationConf.root.empty() ? locationConf.root : serverConf.root;

            // Supprimer le préfixe location (si présent) de l'URI
            std::string relativePath;
            if (!locationConf.path.empty() && req.uri.find(locationConf.path) == 0)
                relativePath = req.uri.substr(locationConf.path.size());
            else if (!req.uri.empty() && req.uri[0] == '/')
                relativePath = req.uri.substr(1);
            else
                relativePath = req.uri;

            if (relativePath.empty() || relativePath == "/") {
                // fallback index
                if (!locationConf.indexFiles.empty())
                    relativePath = locationConf.indexFiles[0];
                else if (!serverConf.indexFiles.empty())
                    relativePath = serverConf.indexFiles[0];
                else
                    relativePath = "index.html";
            }

            std::string filePath = root + "/" + relativePath;

            // DEBUG: log path resolution
            std::cerr << "[DEBUG] root='" << root << "' relativePath='" << relativePath << "' uri='" << req.uri << "' filePath='" << filePath << "'\n";

            // DEBUG: log the candidate filePath(s) we will test
            std::cerr << "responseBuilder: candidate filePath='" << filePath << "'\n";

            // If the constructed filePath does not exist, try alternate behavior:
            // some configurations expect `root` to be the site root and the
            // request URI appended (like nginx). So try root + req.uri as
            // a fallback if the primary path doesn't exist.
            struct stat st;
            if (stat(filePath.c_str(), &st) != 0) {
                // try alternate path: root + req.uri (strip leading '/')
                std::string uriPath = req.uri;
                if (!uriPath.empty() && uriPath[0] == '/') uriPath = uriPath.substr(1);
                std::string altPath = root + "/" + uriPath;
                std::cerr << "responseBuilder: trying altPath='" << altPath << "'\n";
                if (stat(altPath.c_str(), &st) == 0) {
                    filePath = altPath;
                }
            }

            // Si filePath est un dossier et autoindex est activé, générer l'index
            if (stat(filePath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
                if (locationConf.autoindex) {
                    // Générer autoindex HTML via helper
                    std::string ai = generateAutoindexHTML(filePath, req.uri);
                    if (!ai.empty()) {
                        body = ai;
                        headers = "Content-Type: text/html; charset=UTF-8\r\n";
                        headers += "Content-Length: " + ftToString(body.size()) + "\r\n";
                    } else {
                        // cannot read dir -> fallback 403
                        statusLine = "HTTP/1.1 403 Forbidden\r\n";
                        body = "<h1>403 Forbidden</h1>";
                        headers = "Content-Length: " + ftToString(body.size()) + "\r\n";
                        headers += "Content-Type: text/html; charset=UTF-8\r\n";
                    }
                } else {
                    // autoindex off -> try index fallback
                    std::string idxPath;
                    if (!locationConf.indexFiles.empty()) idxPath = root + "/" + locationConf.indexFiles[0];
                    else if (!serverConf.indexFiles.empty()) idxPath = root + "/" + serverConf.indexFiles[0];
                    else idxPath = root + "/index.html";

                    body = ftReadFile(idxPath);
                    if (body.empty()) {
                        statusLine = "HTTP/1.1 403 Forbidden\r\n";
                        body = "<h1>403 Forbidden</h1>";
                        headers = "Content-Length: " + ftToString(body.size()) + "\r\n";
                        headers += "Content-Type: text/html; charset=UTF-8\r\n";
                    } else {
                        std::string contentType = getMimeType(idxPath);
                        headers = "Content-Type: " + contentType + "; charset=UTF-8\r\n";
                        headers += "Content-Length: " + ftToString(body.size()) + "\r\n";
                    }
                }
            } else {
                body = ftReadFile(filePath);
                std::string contentType = getMimeType(filePath);
                headers = "Content-Type: " + contentType + "; charset=UTF-8\r\n";
                headers += "Content-Length: " + ftToString(body.size()) + "\r\n";
            }
        }
    }
    catch (...)
    {
        statusLine = "HTTP/1.1 404 Not Found\r\n";
        body = "<h1>404 Not Found</h1>";
        headers = "Content-Length: " + ftToString(body.size()) + "\r\n";
        headers += "Content-Type: text/html; charset=UTF-8\r\n";
    }

    std::ostringstream response;
    response << statusLine << headers << "\r\n";
    response.write(body.data(), body.size());
    return response.str();
}*/





