#ifndef CONFIG_ERROR_HPP
#define CONFIG_ERROR_HPP

#include <exception>
#include <string>

class ConfigError : public std::exception
{
    std::string _msg;

public:
    explicit ConfigError(const std::string &msg) : _msg(msg) {}
    virtual ~ConfigError() throw() {}

    const char *what() const throw()
    {
        return _msg.c_str();
    }
};

#endif
