/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   extractFilename.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/21 15:02:30 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/21 15:02:31 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/utils.hpp"

std::string extractFilename(const std::string &part)
{
    size_t pos = part.find("filename=\"");
    if (pos == std::string::npos) return "";

    pos += 10;
    size_t end = part.find("\"", pos);
    return (part.substr(pos, end - pos));
}