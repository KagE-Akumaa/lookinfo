#pragma once

#include "IService.hpp"
#include <filesystem>
#include <optional>
#include <unordered_map>

struct MemoryInfo {
        long mem_total_kb;
        long mem_available_kb;
};

class MemoryService : public IService {
      private:
        std::filesystem::path memory_path;
        std::optional<std::string> fileHandler();

        std::unordered_map<std::string, std::string>
        memoryParser(const std::string &memory_file);

      public:
        MemoryService(std::filesystem::path memory_path);

        std::optional<MemoryInfo> getMemoryInfo();

        bool update() override;
};
