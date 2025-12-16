#include "../include/utils.hpp"

std::string normalizeSpaces(const std::string &line)
{
    std::string result;
    bool inSpace = false;

    for (std::string::const_iterator it = line.begin(); it != line.end(); ++it)
    {
        char c = *it;
        if (c == ' ' || c == '\t')
        {
            if (!inSpace)
            {
                result += ' ';
                inSpace = true;
            }
        }
        else
        {
            result += c;
            inSpace = false;
        }
    }

    return result;
}

