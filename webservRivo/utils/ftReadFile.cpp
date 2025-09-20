/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ftReadFile.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:31:19 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/12 16:46:37 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/utils.hpp"

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