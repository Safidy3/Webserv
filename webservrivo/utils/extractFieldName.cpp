#include "../include/utils.hpp"

std::string extractFieldName(const std::string &part)
{
    size_t pos = part.find("name=\"");
    if (pos == std::string::npos)
        return "";

    size_t start = pos + 6;
    size_t end = part.find("\"", start);
    if (end == std::string::npos)
        return "";

    return part.substr(start, end - start);
}
