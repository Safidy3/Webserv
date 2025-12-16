#include "../include/utils.hpp"

std::vector<std::string> ftSplit(const std::string &input, const std::string &delimiter)
{
    std::vector<std::string> parts;
    size_t start = 0, end = 0;

    while ((end = input.find(delimiter, start)) != std::string::npos) {
        std::string part = input.substr(start, end - start);
        if (!part.empty() && part != "\r\n")
            parts.push_back(part);
        start = end + delimiter.size();
    }
    return parts;
}
