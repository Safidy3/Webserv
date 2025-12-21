/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiTimeout.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/21 13:08:13 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/21 13:08:14 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
