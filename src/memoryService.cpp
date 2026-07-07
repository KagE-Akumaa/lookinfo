#include "memoryService.hpp"
#include "fileHandler.hpp"
#include "format"
#include "fstream"
#include <cstring>
#include <iostream>

MemoryService::MemoryService(std::filesystem::path memory_path) {
        this->memory_path = memory_path;
}

std::optional<std::string> MemoryService::fileHandler() {
        std::string MEMORY_PATH = "/proc/meminfo";

        // Step -1 Open the file
        std::ifstream file(MEMORY_PATH);

        if (!file.is_open()) {
                std::cerr << "Error opening the file" << std::strerror(errno)
                          << std::endl;
                return std::nullopt;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        return buffer.str();
}
std::unordered_map<std::string, std::string>
MemoryService::memoryParser(const std::string &memory_file) {
        std::unordered_map<std::string, std::string> memory_map;

        std::stringstream ss(memory_file);
        std::string line{};

        while (std::getline(ss, line)) {
                // upto : that is the key

                size_t keyPos = line.find(':');

                if (keyPos == std::string::npos) {
                        continue;
                }

                std::string_view key = std::string_view(line).substr(0, keyPos);
                std::string valuePart(line.substr(keyPos + 1));

                std::stringstream valueStream(valuePart);

                std::string number{}, unit{};

                valueStream >> number >> unit;

                memory_map.emplace(key, number);
        }

        return memory_map;
}
std::optional<MemoryInfo> MemoryService::getMemoryInfo() {
        auto file = fileHandler();

        if (file == std::nullopt) {
                std::cerr << "Error opening Memory file" << std::endl;
                return std::nullopt;
        }

        std::unordered_map<std::string, std::string> parsedValues =
            memoryParser(*file);

        std::string total_memory{}, available_memory{};

        auto total = parsedValues.find("MemTotal");

        if (total == parsedValues.end()) {
                return std::nullopt;
        }

        total_memory = total->second;

        auto available = parsedValues.find("MemAvailable");

        if (available == parsedValues.end()) {
                return std::nullopt;
        }
        available_memory = available->second;

        MemoryInfo mem_info;

        mem_info.mem_total_kb = std::stoll(total_memory);
        mem_info.mem_available_kb = std::stoll(available_memory);

        return mem_info;
}
bool MemoryService::update() {
        auto mem_info = getMemoryInfo();

        if (mem_info == std::nullopt) {
                std::cerr << "Error obtaining memory information" << std::endl;
                return false;
        }

        // Add the string to display on memory_path

        long used_kb = mem_info->mem_total_kb - mem_info->mem_available_kb;

        double used_gb = used_kb / (1024.0 * 1024.0);

        double total_gb = mem_info->mem_total_kb / (1024.0 * 1024.0);
        std::string text = "🧠 " + std::format("{:.1f}", used_gb) + "G / " +
                           std::format("{:.1f}", total_gb) + "G";
        if (!fileWriter(memory_path, text)) {

                return false;
        }
        return true;
}
