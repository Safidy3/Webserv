#include "../include/utils.hpp"

std::vector<std::string> splitParts(const std::string &body, const std::string &delimiter)
{
    std::vector<std::string> parts;
    size_t start = 0, end = 0;

    while ((end = body.find(delimiter, start)) != std::string::npos) {
        std::string part = body.substr(start, end - start);
        if (!part.empty() && part != "\r\n")
            parts.push_back(part);
        start = end + delimiter.size();
    }
    return parts;
}
