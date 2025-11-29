#include "../include/utils.hpp"

std::string extractFilename(const std::string &part)
{
    size_t pos = part.find("filename=\"");
    if (pos == std::string::npos) return "";

    pos += 10;
    size_t end = part.find("\"", pos);
    return (part.substr(pos, end - pos));
}
