#pragma once

#include <filesystem>
#include <string>

// NOTE: This function return true on success and false otherwise and is
// responsible for Opening the file from the file_path, if not present cerating
// one and then appending the data to it
bool fileWriter(const std::filesystem::path &file_path,

                const std::string &data);
