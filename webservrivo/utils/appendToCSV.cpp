/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   appendToCSV.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/21 15:01:44 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/21 15:01:47 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/utils.hpp"
#include <iostream>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

static std::string sanitizeString(const std::string &s)
{
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '\n' || c == '\r' || c == ';')
            out += ' ';
        else
            out += c;
    }
    return out;
}

static std::string getFieldValue(const std::map<std::string, std::string> &fieldsMap, const std::string &key)
{
    std::map<std::string, std::string>::const_iterator it = fieldsMap.find(key);
    if (it != fieldsMap.end())
        return sanitizeString(it->second);
    return std::string();
}

void appendToCSV(const std::map<std::string, std::string> &fields, const std::string &csvPath, const std::string &filename)
{
    int fd = open(csvPath.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0) {
        std::cerr << "appendToCSV: impossible d'ouvrir le fichier " << csvPath << " errno=" << errno << "\n";
        return;
    }

    if (flock(fd, LOCK_EX) != 0) {
        std::cerr << "appendToCSV: flock failed errno=" << errno << "\n";
    }

    struct stat st;
    bool empty = false;
    if (fstat(fd, &st) == 0) {
        empty = (st.st_size == 0);
    }

    std::string line;
    if (empty) {
        line += "FirstName;Name;phone;email;information;filename\n";
    }

    line += getFieldValue(fields, "FirstName"); line += ";";
    line += getFieldValue(fields, "Name"); line += ";";
    line += getFieldValue(fields, "phone"); line += ";";
    line += getFieldValue(fields, "email"); line += ";";
    line += getFieldValue(fields, "information"); line += ";";
    line += sanitizeString(filename);
    line += "\n";

    const char *buf = line.c_str();
    ssize_t toWrite = static_cast<ssize_t>(line.size());
    ssize_t written = 0;
    while (written < toWrite) {
        ssize_t w = write(fd, buf + written, toWrite - written);
        if (w <= 0) {
            std::cerr << "appendToCSV: write returned <= 0, aborting\n";
            break;
        }
        written += w;
    }

    flock(fd, LOCK_UN);
    close(fd);
}
