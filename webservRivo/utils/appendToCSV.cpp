#include "../include/utils.hpp"
#include <iostream>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

// Helper utilities at file scope (compatible with C++98)
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

// Write CSV line using O_APPEND and flock to reduce race conditions
void appendToCSV(const std::map<std::string, std::string> &fields,
                 const std::string &csvPath,
                 const std::string &filename)
{
    // Open file descriptor with append/create mode and permissions 0644
    int fd = open(csvPath.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0) {
        std::cerr << "appendToCSV: impossible d'ouvrir le fichier " << csvPath << " errno=" << errno << "\n";
        return;
    }

    // Acquire an exclusive lock while we potentially write header + line
    if (flock(fd, LOCK_EX) != 0) {
        std::cerr << "appendToCSV: flock failed errno=" << errno << "\n";
        // proceed without lock if flock not supported, but keep fd
    }

    // Check if file is empty (size == 0) to write header once
    struct stat st;
    bool empty = false;
    if (fstat(fd, &st) == 0) {
        empty = (st.st_size == 0);
    }

    std::string line;
    if (empty) {
        line += "FirstName;Name;sex;BirthDay;status;phone;email;information;filename\n";
    }

    line += getFieldValue(fields, "FirstName"); line += ";";
    line += getFieldValue(fields, "Name"); line += ";";
    line += getFieldValue(fields, "sex"); line += ";";
    line += getFieldValue(fields, "BirthDay"); line += ";";
    line += getFieldValue(fields, "status"); line += ";";
    line += getFieldValue(fields, "phone"); line += ";";
    line += getFieldValue(fields, "email"); line += ";";
    line += getFieldValue(fields, "information"); line += ";";
    line += sanitizeString(filename);
    line += "\n";

    // Write atomically using write(2) on the FD opened with O_APPEND
    const char *buf = line.c_str();
    ssize_t toWrite = static_cast<ssize_t>(line.size());
    ssize_t written = 0;
    while (written < toWrite) {
        ssize_t w = write(fd, buf + written, toWrite - written);
        if (w < 0) {
            if (errno == EINTR) continue;
            std::cerr << "appendToCSV: write failed errno=" << errno << "\n";
            break;
        }
        written += w;
    }

    // Unlock and close
    flock(fd, LOCK_UN);
    close(fd);
}
