/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ftTrim.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/21 15:03:13 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/21 15:03:14 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/utils.hpp"
#include <string>
#include <cctype>

std::string ftTrim(const std::string &s)
{
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
        start++;

    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
        end--;

    return s.substr(start, end - start);
}
