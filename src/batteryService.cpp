#include "batteryService.hpp"
#include <cstring>
#include <fileHandler.hpp>
#include <fstream>
#include <iostream>

BatteryService::BatteryService(std::filesystem::path battery_path) {
        this->battery_path = battery_path;
}
std::optional<std::string> BatteryService::fileHandler() {
        //
        std::string BATTERY_PATH = "/sys/class/power_supply/BAT1/uevent";

        // Step -1 Open the file
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
                // upto == that is the key
        }
        std::cout << battery_file << std::endl;

        return battery_map;
}
std::optional<BatteryInfo> BatteryService::getBatteryInfo() {
        //
        auto file = fileHandler();

        if (file == std::nullopt) {
                std::cerr << "Error opening Battery file" << std::endl;

                return std::nullopt;
        }

        std::unordered_map<std::string, std::string> parsedValues =
            batteryParser(*file);

        std::string capacity = parsedValues["POWER_SUPPLY_CAPACITY"];
        std::string status = parsedValues["POWER_SUPPLY_STATUS"];

        BatteryInfo b_info;
        // b_info.percentage = std::stoi(capacity);
        b_info.status = status;

        return b_info;
}

bool BatteryService::update() {
        //
        auto b_info = getBatteryInfo();

        if (b_info == std::nullopt) {
                std::cerr << "Error obtaining battery information" << std::endl;
                return false;
        }

        std::string text =
            "🔋 " + std::to_string(b_info->percentage) + "% " + b_info->status;

        if (!fileWriter(battery_path, text)) {
                return false;
        }
        return true;
}
