/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ftReadFile.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:31:19 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/21 15:02:48 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/utils.hpp"
#include <sys/stat.h>
#include <fstream>
#include <sstream>

bool file_exists(const std::string &path)
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool file_is_redable(const std::string& cheminFichier)
{
    struct stat st;

    if (stat(cheminFichier.c_str(), &st) != 0) {
        std::cout << "Le fichier n'existe pas.\n";
        return false;
    }

    if (access(cheminFichier.c_str(), R_OK) != 0) {
        std::cout << "Le fichier existe mais il n'est pas lisible.\n";
        return false;
    }

    return true;
}


std::string ftReadFile(const std::string &path)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Cannot open file: " + path);

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}


