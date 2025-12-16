#ifndef CGI_TIMEOUT_HPP
#define CGI_TIMEOUT_HPP

#include <stdexcept>
#include <string>

class CgiTimeout : public std::runtime_error {
public:
    CgiTimeout() 
        : std::runtime_error("CGI Timeout") {}
};

#endif
