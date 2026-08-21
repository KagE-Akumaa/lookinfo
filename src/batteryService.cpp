#include "batteryService.hpp"
#include <cstring>
#include <fileHandler.hpp>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <utility>

BatteryService::BatteryService(std::filesystem::path batteryOutputPath) {
        this->batteryOutputPath = batteryOutputPath;
        this->batteryPath = "/sys/class/power_supply";
}
std::optional<std::string>
BatteryService::fileHandler(const std::string &dirName) {

        // Step -1 Open the file
        std::filesystem::path BATTERY_PATH =
            std::filesystem::path(dirName) / "uevent";
        std::ifstream file(BATTERY_PATH);

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
BatteryService::batteryParser(const std::string &battery_file) {
        std::unordered_map<std::string, std::string> battery_map;

        std::stringstream ss(battery_file);
        std::string line;

        while (std::getline(ss, line)) {
                // upto = that is the key
                size_t pos = line.find('=');

                if (pos == std::string::npos) {
                        continue;
                }
                std::string_view key = std::string_view(line).substr(0, pos);

                std::string_view value = std::string_view(line).substr(pos + 1);

                battery_map.emplace(key, value);
        }

        return battery_map;
}
// NOTE: There is no support for multiple batteries the first one recognized by
// directory_iterator with type Battery is considered
std::optional<std::pair<std::string, bool>>
BatteryService::findBatteryDirectory(std::filesystem::path &batteryPath) {
        // Goal to find a directory where the type is battery

        // Helper function to get the filecontent without any white space
        auto trimSpace = [](std::string &typeContent) -> std::string {
                auto not_space = [](unsigned char c) {
                        return !std::isspace(c);
                };

                typeContent.erase(typeContent.begin(),
                                  std::ranges::find_if(typeContent.begin(),
                                                       typeContent.end(),
                                                       not_space));

                typeContent.erase(std::ranges::find_if(typeContent.rbegin(),
                                                       typeContent.rend(),
                                                       not_space)
                                      .base(),
                                  typeContent.end());

                return typeContent;
        };
        // Step 1 Get the directory inside battery path
        std::error_code ec{};

        auto it = std::filesystem::directory_iterator(batteryPath, ec);
        if (ec) {
                // Returning nullopt signifies a hard directory-read failure
                return std::nullopt;
        }

        for (auto const &entry : it) {
                // Step 1 Get the directory name
                std::filesystem::path dirName = entry.path();
                // Step 2 Make the path for type attribute
                std::filesystem::path typeFilePath = dirName / "type";

                // Step 3 Read the type file
                std::ifstream file(typeFilePath,
                                   std::ios::in | std::ios::binary);
                if (!file.is_open()) {
                        // NOTE: If the file does not exist skip it
                        if (!std::filesystem::exists(typeFilePath)) {
                                continue;
                        }
                        // Any other OS error reading the file returns a hard
                        // failure
                        return std::nullopt;
                }
                std::string typeContent{};

                if (std::getline(file, typeContent)) {
                        // Step 4 Trim white spaces from the type file
                        if (trimSpace(typeContent) == "Battery") {
                                // Return path and found=true status
                                return std::make_pair(dirName.string(), true);
                        }
                }
        }
        // No Battery found but execution is successful (found = false)
        return std::make_pair("", false);
}
std::optional<BatteryInfo> BatteryService::getBatteryInfo() {

        BatteryInfo b_info{};
        auto dir = findBatteryDirectory(batteryPath);
        if (dir == std::nullopt) {
                std::cerr << "Error opening Battery Directory" << std::endl;
                return std::nullopt;
        } else if (dir->first.empty() && dir->second == false) {
                // Means there is no battery
                // Empty object
                return b_info;
        }
        // NOTE: Now fileHandler can have the path to the directory
        auto file = fileHandler(dir->first);

        if (file == std::nullopt) {
                std::cerr << "Error opening Battery file" << std::endl;

                return std::nullopt;
        }

        std::unordered_map<std::string, std::string> parsedValues =
            batteryParser(*file);

        std::string capacity{}, status{};

        auto cap = parsedValues.find("POWER_SUPPLY_CAPACITY");
        if (cap == parsedValues.end()) {
                return std::nullopt;
        }

        capacity = cap->second;

        auto stat = parsedValues.find("POWER_SUPPLY_STATUS");
        if (stat == parsedValues.end()) {
                return std::nullopt;
        }

        status = stat->second;

        // NOTE: stoi() can throw if the capacity is like "abs"
        b_info.percentage = std::stoi(capacity);
        b_info.status = status;

        return b_info;
}

bool BatteryService::update() {
        auto b_info = getBatteryInfo();

        // 1. Hard failure check (e.g., directory errors, missing keys)
        if (b_info == std::nullopt) {
                std::cerr << "Error obtaining battery information" << std::endl;
                return false;
        }

        // 2. SUCCESSFUL execution but NO BATTERY physically present
        if (b_info->empty()) {
                // Early return true because this isn't a program error.
                // Option A: Write blank space to the output path so the UI
                // clears up.
                fileWriter(batteryOutputPath, "");
                return true;
        }

        // 3. Normal execution with battery data present
        std::string text =
            "🔋 " + std::to_string(b_info->percentage) + "% " + b_info->status;

        if (!fileWriter(batteryOutputPath, text)) {
                return false;
        }
        return true;
}
