#pragma once

#include <chrono>
#include <filesystem>

struct WeatherConfig {
        bool enabled{false};
        std::chrono::seconds interval{1800};
        double latitude{};
        double longitude{};
};

struct BatteryConfig {
        bool enabled{true};
        std::chrono::seconds interval{30};
};

struct MemoryConfig {
        bool enabled{true};
        std::chrono::seconds interval{10};
};

struct WallpaperConfig {
        bool enabled{false};
        std::chrono::seconds interval{300};
        std::filesystem::path directory{};
};

struct QuotesConfig {
        bool enabled{false};
        std::chrono::seconds interval{3600};
};

struct LookInfoConfig {
        std::filesystem::path cacheDirectory{};

        WeatherConfig weather{};
        BatteryConfig battery{};
        MemoryConfig memory{};
        WallpaperConfig wallpaper{};
        QuotesConfig quotes{};
};
