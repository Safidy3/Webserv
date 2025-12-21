/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   extractFileContent.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/21 15:02:24 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/21 15:02:25 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/utils.hpp"

std::string extractFileContent(const std::string &part)
{
    size_t pos = part.find("\r\n\r\n");
    if (pos == std::string::npos) return "";
    pos += 4;
    size_t end = part.rfind("\r\n");
    return (part.substr(pos, end - pos));
}
