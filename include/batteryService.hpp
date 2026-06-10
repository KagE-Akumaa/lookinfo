#pragma once

#include "IService.hpp"
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
struct BatteryInfo {
        std::string status;
        int percentage;
};
class BatteryService : public IService {
      private:
        std::filesystem::path battery_path;
        std::optional<std::string> fileHandler();

        std::unordered_map<std::string, std::string>
        batteryParser(const std::string &battery_file);

      public:
        BatteryService(std::filesystem::path battery_path);
        std::optional<BatteryInfo> getBatteryInfo();

        bool update() override;
};
