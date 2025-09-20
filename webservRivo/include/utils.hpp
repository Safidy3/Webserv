/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/29 16:22:27 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/12 19:39:58 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <stdexcept>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <cstring>

template <typename T>
std::string ftToString(T value)
{
    std::ostringstream ss;
    ss << value;
    return (ss.str());
}

int ftToInt(const std::string &s);
std::string ftStrdup(const char* s);
std::string ftReadFile(const std::string &path);
std::vector<std::string> ftSplitStr(const std::string& str, const std::string& delimiter);

#endif
