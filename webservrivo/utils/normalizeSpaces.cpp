/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   normalizeSpaces.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/21 15:05:43 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/21 15:05:45 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

