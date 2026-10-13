#include "configLoader.hpp"

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <toml++/toml.hpp>

namespace {

using Table = toml::table;

[[noreturn]] void invalid(std::string_view setting, std::string_view reason) {
        throw std::runtime_error("Invalid configuration setting '" +
                                 std::string(setting) +
                                 "': " + std::string(reason));
}

// Read a value only when it is present, but never silently accept a wrong TOML
// type. This keeps all optional settings consistent across sections.
template <typename T>
std::optional<T> readOptional(const Table *table, std::string_view section,
                              std::string_view key) {
        if (table == nullptr) {
                return std::nullopt;
        }

        const auto *node = table->get(key);
        if (node == nullptr) {
                return std::nullopt;
        }

        const auto value = node->value<T>();
        if (!value) {
                invalid(std::string(section) + "." + std::string(key),
                        "has the wrong type");
        }
        return *value;
}

const Table *readSection(const Table &config, std::string_view section) {
        const auto *node = config.get(section);
        if (node == nullptr) {
                return nullptr;
        }

        const auto *table = node->as_table();
        if (table == nullptr) {
                invalid(section, "must be a table");
        }
        return table;
}

void readEnabled(const Table *table, std::string_view section, bool &enabled) {
        if (const auto value = readOptional<bool>(table, section, "enabled")) {
                enabled = *value;
        }
}

void readInterval(const Table *table, std::string_view section,
                  std::chrono::seconds &interval) {
        if (const auto value =
                readOptional<int64_t>(table, section, "interval_seconds")) {
                if (*value <= 0) {
                        invalid(std::string(section) + ".interval_seconds",
                                "must be greater than zero");
                }
                interval = std::chrono::seconds{*value};
        }
}

std::filesystem::path readPath(const Table *table, std::string_view section,
                               std::string_view key) {
        const auto value = readOptional<std::string>(table, section, key);
        if (!value) {
                return {};
        }
        if (value->empty()) {
                invalid(std::string(section) + "." + std::string(key),
                        "must not be empty");
        }
        return std::filesystem::path{*value};
}

void readCoordinates(const Table *table, WeatherConfig &weather) {
        if (const auto latitude =
                readOptional<double>(table, "weather", "latitude")) {
                if (!std::isfinite(*latitude) || *latitude < -90.0 ||
                    *latitude > 90.0) {
                        invalid("weather.latitude",
                                "must be finite and between -90 and 90");
                }
                weather.latitude = *latitude;
        }

        if (const auto longitude =
                readOptional<double>(table, "weather", "longitude")) {
                if (!std::isfinite(*longitude) || *longitude < -180.0 ||
                    *longitude > 180.0) {
                        invalid("weather.longitude",
                                "must be finite and between -180 and 180");
                }
                weather.longitude = *longitude;
        }
}

} // namespace

LookInfoConfig ConfigLoader::load(const std::filesystem::path &configPath) {
        if (configPath.empty()) {
                throw std::invalid_argument(
                    "Configuration path must not be empty");
        }

        std::error_code filesystemError;
        const auto status =
            std::filesystem::status(configPath, filesystemError);
        if (filesystemError) {
                throw std::runtime_error(
                    "Unable to access configuration file '" +
                    configPath.string() + "': " + filesystemError.message());
        }
        if (!std::filesystem::exists(status)) {
                throw std::runtime_error(
                    "Configuration file does not exist: '" +
                    configPath.string() + "'");
        }
        if (!std::filesystem::is_regular_file(status)) {
                throw std::runtime_error(
                    "Configuration path is not a regular file: '" +
                    configPath.string() + "'");
        }

        toml::table config;
        try {
                config = toml::parse_file(configPath.string());
        } catch (const toml::parse_error &error) {
                throw std::runtime_error(
                    "Failed to parse config file '" + configPath.string() +
                    "': " + std::string(error.description()));
        } catch (const std::exception &error) {
                throw std::runtime_error("Failed to read config file '" +
                                         configPath.string() +
                                         "': " + error.what());
        }

        LookInfoConfig result{};
        const auto *general = readSection(config, "general");
        const auto *weather = readSection(config, "weather");
        const auto *battery = readSection(config, "battery");
        const auto *memory = readSection(config, "memory");
        const auto *wallpaper = readSection(config, "wallpaper");
        const auto *quotes = readSection(config, "quotes");

        if (const auto cacheDirectory =
                readPath(general, "general", "cache_dir");
            !cacheDirectory.empty()) {
                result.cacheDirectory = cacheDirectory;
        }

        readEnabled(weather, "weather", result.weather.enabled);
        readInterval(weather, "weather", result.weather.interval);
        readCoordinates(weather, result.weather);

        readEnabled(battery, "battery", result.battery.enabled);
        readInterval(battery, "battery", result.battery.interval);

        readEnabled(memory, "memory", result.memory.enabled);
        readInterval(memory, "memory", result.memory.interval);

        readEnabled(wallpaper, "wallpaper", result.wallpaper.enabled);
        readInterval(wallpaper, "wallpaper", result.wallpaper.interval);
        const auto wallpaperDirectory =
            readPath(wallpaper, "wallpaper", "directory");
        if (!wallpaperDirectory.empty()) {
                result.wallpaper.directory = wallpaperDirectory;
        }

        readEnabled(quotes, "quotes", result.quotes.enabled);
        readInterval(quotes, "quotes", result.quotes.interval);

        return result;
}
