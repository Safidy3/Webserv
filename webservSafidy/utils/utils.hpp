/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: safandri <safandri@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/29 16:22:27 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/20 15:15:53 by safandri         ###   ########.fr       */
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
    std::ostringstream os;
    os << value;
    return os.str();
}
int ftToInt(const std::string &s);
void set_nonblocking(int fd);
std::string ftStrdup(const char* s);
std::string ftReadFile(const std::string &path);
std::vector<std::string> ftSplitStr(const std::string& str, const std::string& delimiter);
bool ftFileExists(const std::string &path);
bool ftIsFile(const std::string &path);
bool ftIsDirectory(const std::string &path);

#endif
