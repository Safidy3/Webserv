/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configError.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/21 13:08:16 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/21 13:08:17 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
