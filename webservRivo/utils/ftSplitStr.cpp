/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ftSplitStr.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:36:25 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/08 19:46:41 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/utils.hpp"

std::vector<std::string> ftSplitStr(const std::string& str, const std::string& delimiter)
{
    std::vector<std::string> output;
    std::string::size_type start = 0;
    std::string::size_type end;

    while ((end = str.find(delimiter, start)) != std::string::npos)
    {
        if (end != start)
            output.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
    }

    if (start < str.size())
        output.push_back(str.substr(start));

    return (output);
}
