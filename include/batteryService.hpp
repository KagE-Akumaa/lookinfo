#pragma once

#include "IService.hpp"
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

struct BatteryInfo {
        std::string status{};
        int percentage{-1};

        bool empty() const { return status.empty() && percentage == -1; }
};

class BatteryService : public IService {
      private:
        std::filesystem::path batteryPath;
        std::filesystem::path batteryOutputPath;
        std::optional<std::string> fileHandler(const std::string &dirName);
        std::optional<std::pair<std::string, bool>>
        findBatteryDirectory(std::filesystem::path &batteryPath);

        std::unordered_map<std::string, std::string>
        batteryParser(const std::string &battery_file);

      public:
        BatteryService(std::filesystem::path battery_path);
        std::optional<BatteryInfo> getBatteryInfo();

        bool update() override;
};
