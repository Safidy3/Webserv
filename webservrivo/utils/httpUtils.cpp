#include "../include/utils.hpp"
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <dirent.h>
#include <string>
#include <vector>

std::string normalizeRelativePath(const std::string &relative)
{
    std::vector<std::string> parts;
    std::istringstream ss(relative);
    std::string token;
    while (std::getline(ss, token, '/')) {
        if (token.empty() || token == ".") continue;
        if (token == "..") {
            if (!parts.empty()) parts.pop_back();
        } else parts.push_back(token);
    }
    std::string norm;
    for (size_t i = 0; i < parts.size(); ++i) {
        norm += "/" + parts[i];
    }
    if (norm.empty()) norm = "/";
    return norm;
}

bool isPathInsideRoot(const std::string &root, const std::string &target, std::string &outCanonicalTarget)
{
    char rootBuf[PATH_MAX];
    if (!realpath(root.c_str(), rootBuf)) return false;
    std::string canonRoot(rootBuf);

    char targetBuf[PATH_MAX];
    if (realpath(target.c_str(), targetBuf)) {
        outCanonicalTarget = std::string(targetBuf);
    } else {
        // target may not exist yet; build path by concatenating
        outCanonicalTarget = canonRoot + normalizeRelativePath(target);
    }
    return outCanonicalTarget.find(canonRoot) == 0;
}

std::string generateAutoindexHTML(const std::string &dirPath, const std::string &uri)
{
    DIR *dir = opendir(dirPath.c_str());
    if (!dir) return std::string();
    std::ostringstream oss;
    oss << "<html><head><title>Index of " << uri << "</title></head><body>";
    oss << "<h1>Index of " << uri << "</h1><ul>";
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name(entry->d_name);
        if (name == "." || name == "..") continue;
        oss << "<li><a href=\"" << name << "\">" << name << "</a></li>";
    }
    oss << "</ul></body></html>";
    closedir(dir);
    return oss.str();
}

std::string parseCGIStatusFromHeaders(const std::string &headers)
{
    std::istringstream hh(headers);
    std::string line;
    while (std::getline(hh, line)) {
        if (!line.empty() && line[line.size()-1] == '\r') line.erase(line.size()-1);
        if (line.size() >= 7 && line.substr(0,7) == "Status:") {
            std::string status = line.substr(7);
            size_t s = status.find_first_not_of(" \t");
            if (s != std::string::npos) status = status.substr(s);
            return std::string("HTTP/1.1 ") + status + "\r\n";
        }
    }
    return std::string();
}

bool checkClientMaxBodySize(size_t contentLength, size_t clientMaxBodySize)
{
    if (clientMaxBodySize == 0) return false;
    return contentLength > clientMaxBodySize;
}

std::string dechunkBody(const std::string &chunkedBody)
{
    std::string out;
    size_t pos = 0;
    while (pos < chunkedBody.size()) {
        // find line with chunk size
        size_t lineEnd = chunkedBody.find("\r\n", pos);
        if (lineEnd == std::string::npos) return std::string();
        std::string sizeLine = chunkedBody.substr(pos, lineEnd - pos);
        // chunk size may contain extensions after ';'
        size_t semi = sizeLine.find(';');
        std::string sizeHex = (semi == std::string::npos) ? sizeLine : sizeLine.substr(0, semi);
        // parse hex
        unsigned long chunkSize = strtoul(sizeHex.c_str(), NULL, 16);
        pos = lineEnd + 2;
        if (chunkSize == 0) {
            // consume trailing CRLF after zero chunk and optional trailers
            // look for double CRLF marking end of chunks
            size_t endPos = chunkedBody.find("\r\n\r\n", pos);
            if (endPos == std::string::npos) {
                // maybe just CRLF
                if (pos + 2 <= chunkedBody.size()) return out;
                return std::string();
            }
            return out; // finished
        }
        if (pos + chunkSize > chunkedBody.size()) return std::string();
        out.append(chunkedBody.data() + pos, chunkSize);
        pos += chunkSize;
        // next must be CRLF
        if (pos + 2 > chunkedBody.size()) return std::string();
        if (chunkedBody[pos] != '\r' || chunkedBody[pos+1] != '\n') return std::string();
        pos += 2;
    }
    return std::string();
}

bool isCgiRequest(const HttpRequest &req, const LocationConfig &locationConf, std::string &cgiPath)
{
    if (locationConf.cgiExtension.empty())
        return false;
    if (req.uri.size() < locationConf.cgiExtension.size())
        return false;

    if (req.uri.compare(req.uri.size() - locationConf.cgiExtension.size(),
        locationConf.cgiExtension.size(), locationConf.cgiExtension) == 0)
    {
        cgiPath = locationConf.root + req.uri.substr(locationConf.path.size());
        return true;
    }
    return false;
}

std::string resolveFilePath(const HttpRequest &req, const ServerConfig &serverConf, const LocationConfig &locationConf)
{
    std::string root = !locationConf.root.empty() ? locationConf.root : serverConf.root;
    std::string uri = req.uri;

    // Retirer la partie du chemin correspondant à la location
    std::string relativePath;
    if (!locationConf.path.empty() && uri.find(locationConf.path) == 0)
        relativePath = uri.substr(locationConf.path.size());
    else
        relativePath = uri;

    // Nettoyer les / superflus
    if (!relativePath.empty() && relativePath[0] == '/')
        relativePath.erase(0, 1);

    std::string filePath = root + "/" + relativePath;

    // Supprimer les // doublons au cas où
    while (filePath.find("//") != std::string::npos)
        filePath.replace(filePath.find("//"), 2, "/");

    return filePath;
}