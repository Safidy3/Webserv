#include "../include/utils.hpp"

bool isAbsoluteURL(const std::string &p)
{
    return (p.rfind("http://", 0) == 0 || p.rfind("https://", 0) == 0);
}
