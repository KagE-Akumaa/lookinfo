#include "fileHandler.hpp"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

bool fileWriter(const std::filesystem::path &file_path,
                const std::string &data) {
        //
        // O_CREAT - if not present create one, O_WRONLY - Only for writing,
        // O_TRUNC - if file exist and is opened successfully truncate it's
        // length to 0 0600 is the permission for this file
        int fd = open(file_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0600);

        if (fd == -1) {
                std::cerr << "Error opening the file " << file_path
                          << std::strerror(errno) << std::endl;
                return false;
        }

        ssize_t bytes = write(fd, data.c_str(), data.size());

        if (bytes == -1) {
                std::cerr << " Failed to write to file" << std::strerror(errno)
                          << std::endl;
                close(fd);
                return false;
        }
        // NOTE: Remeber to close the file descriptor
        close(fd);

        return true;
}
