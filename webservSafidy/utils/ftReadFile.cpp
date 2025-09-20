/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ftReadFile.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: safandri <safandri@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:31:19 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/20 15:11:24 by safandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

std::string ftReadFile(const std::string &path)
{
    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    std::ostringstream fileContent;
    fileContent << file.rdbuf();
    return (fileContent.str());
}