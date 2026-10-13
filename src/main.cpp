#include "batteryService.hpp"
#include "configLoader.hpp"
#include "memoryService.hpp"
#include "pathFinder.hpp"
#include "quoteService.hpp"
#include "scheduler.hpp"
#include "wallpaper.hpp"
#include "weather.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace {

std::filesystem::path expandHome(const std::filesystem::path &path,
                                 const std::filesystem::path &home) {
        const auto pathString = path.string();
        if (pathString == "~") {
                return home;
        }
        if (pathString.starts_with("~/")) {
                return home / pathString.substr(2);
        }
        return path;
}

} // namespace

int main() {
        try {
                char *h = std::getenv("HOME");

                if (h == nullptr) {
                        std::cout << "HOME DOES NOT EXIST" << std::endl;
                        return 1;
                }

                std::filesystem::path home_path = h;
                const std::filesystem::path configPath =
                    home_path / ".config" / "lookinfo" / "config.toml";
                ConfigLoader loader{};

                const LookInfoConfig config = loader.load(configPath);

                std::filesystem::path dirPath;
                if (config.cacheDirectory.empty()) {
                        const auto defaultCacheDirectory = getCacheDirectory();
                        if (!defaultCacheDirectory) {
                                std::cerr << "Failed to get cache directory\n";
                                return 1;
                        }
                        dirPath = *defaultCacheDirectory;
                } else {
                        dirPath = expandHome(config.cacheDirectory, home_path);
                        std::filesystem::create_directories(dirPath);
                }

                const auto weather_path = dirPath / "weather.txt";
                const auto battery_path = dirPath / "battery.txt";
                const auto memory_path = dirPath / "memory.txt";
                const auto wallpaper_path = dirPath / "wallpaper.txt";

                // Step - 4 Initialize the weather service
                std::string weather_api_url =
                    "https://api.open-meteo.com/v1/forecast?latitude=" +
                    std::to_string(config.weather.latitude) +
                    "&longitude=" + std::to_string(config.weather.longitude) +
                    "&daily=temperature_2m_max,temperature_2m_min"
                    "&current=temperature_2m&timezone=auto&forecast_days=1";
                WeatherService weather(weather_path, weather_api_url);

                // Initialize the BatteryService
                BatteryService battery(battery_path);
                // Initialize the MemoryService
                MemoryService memory(memory_path);
                // Initialize the WallpaperService
                WallpaperService wallpaper(
                    expandHome(config.wallpaper.directory, home_path),
                    wallpaper_path);
                Scheduler scheduler;
                if (config.weather.enabled) {
                        scheduler.add(&weather, config.weather.interval);
                }
                if (config.memory.enabled) {
                        scheduler.add(&memory, config.memory.interval);
                }
                if (config.battery.enabled) {
                        scheduler.add(&battery, config.battery.interval);
                }
                if (config.wallpaper.enabled) {
                        scheduler.add(&wallpaper, config.wallpaper.interval);
                }

                scheduler.run();

        } catch (const std::exception &error) {
                std::cerr << "LookInfo startup failed: " << error.what()
                          << '\n';
                return 1;
        }

        return 0;
}
