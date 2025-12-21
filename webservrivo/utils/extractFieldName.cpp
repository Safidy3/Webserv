/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   extractFieldName.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/21 15:02:08 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/21 15:02:09 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
