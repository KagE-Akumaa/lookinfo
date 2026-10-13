#include "configLoader.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

class ConfigLoaderTest : public ::testing::Test {
  protected:
    void SetUp() override {
        const auto unique_id = std::chrono::steady_clock::now()
                                   .time_since_epoch()
                                   .count();
        temporary_directory = std::filesystem::temp_directory_path() /
                              ("lookinfo_config_test_" +
                               std::to_string(unique_id));
        ASSERT_TRUE(std::filesystem::create_directories(temporary_directory));
    }

    void TearDown() override {
        std::filesystem::remove_all(temporary_directory);
    }

    std::filesystem::path writeConfig(const std::string &contents) {
        const auto path = temporary_directory / "config.toml";
        std::ofstream file(path);
        EXPECT_TRUE(file.is_open());
        file << contents;
        return path;
    }

    void expectInvalidConfiguration(const std::string &contents,
                                    const std::string &message) {
        const auto path = writeConfig(contents);
        try {
            ConfigLoader{}.load(path);
            FAIL() << "Expected configuration loading to fail";
        } catch (const std::runtime_error &error) {
            EXPECT_NE(std::string(error.what()).find(message),
                  std::string::npos)
            << error.what();
        }
    }

    std::filesystem::path temporary_directory;
};

TEST_F(ConfigLoaderTest, LoadsAllSupportedValues) {
    const auto path = writeConfig(R"toml(
[general]
cache_dir = "/var/cache/lookinfo"

[weather]
enabled = true
interval_seconds = 1
latitude = -90.0
longitude = 180.0

[battery]
enabled = false
interval_seconds = 7200

[memory]
enabled = false
interval_seconds = 1

[wallpaper]
enabled = true
interval_seconds = 86400
directory = "/pictures/wallpapers"

[quotes]
enabled = true
interval_seconds = 2
)toml");

    const auto config = ConfigLoader{}.load(path);

    EXPECT_EQ(config.cacheDirectory, "/var/cache/lookinfo");
    EXPECT_TRUE(config.weather.enabled);
    EXPECT_EQ(config.weather.interval, std::chrono::seconds{1});
    EXPECT_DOUBLE_EQ(config.weather.latitude, -90.0);
    EXPECT_DOUBLE_EQ(config.weather.longitude, 180.0);
    EXPECT_FALSE(config.battery.enabled);
    EXPECT_EQ(config.battery.interval, std::chrono::seconds{7200});
    EXPECT_FALSE(config.memory.enabled);
    EXPECT_EQ(config.memory.interval, std::chrono::seconds{1});
    EXPECT_TRUE(config.wallpaper.enabled);
    EXPECT_EQ(config.wallpaper.interval, std::chrono::seconds{86400});
    EXPECT_EQ(config.wallpaper.directory, "/pictures/wallpapers");
    EXPECT_TRUE(config.quotes.enabled);
    EXPECT_EQ(config.quotes.interval, std::chrono::seconds{2});
}

TEST_F(ConfigLoaderTest, UsesDefaultsWhenSectionsAndValuesAreMissing) {
    const auto config = ConfigLoader{}.load(writeConfig(""));

    EXPECT_TRUE(config.cacheDirectory.empty());
    EXPECT_FALSE(config.weather.enabled);
    EXPECT_EQ(config.weather.interval, std::chrono::seconds{1800});
    EXPECT_DOUBLE_EQ(config.weather.latitude, 0.0);
    EXPECT_DOUBLE_EQ(config.weather.longitude, 0.0);
    EXPECT_TRUE(config.battery.enabled);
    EXPECT_EQ(config.battery.interval, std::chrono::seconds{30});
    EXPECT_TRUE(config.memory.enabled);
    EXPECT_EQ(config.memory.interval, std::chrono::seconds{10});
    EXPECT_FALSE(config.wallpaper.enabled);
    EXPECT_EQ(config.wallpaper.interval, std::chrono::seconds{300});
    EXPECT_TRUE(config.wallpaper.directory.empty());
    EXPECT_FALSE(config.quotes.enabled);
    EXPECT_EQ(config.quotes.interval, std::chrono::seconds{3600});
}

TEST_F(ConfigLoaderTest, IgnoresUnknownSectionsAndKeys) {
    const auto config = ConfigLoader{}.load(writeConfig(R"toml(
[scheduler]
default_interval_seconds = 60

[weather]
enabled = true
unexpected = "ignored"
)toml"));

    EXPECT_TRUE(config.weather.enabled);
    EXPECT_EQ(config.weather.interval, std::chrono::seconds{1800});
}

TEST_F(ConfigLoaderTest, RejectsEmptyConfigurationPath) {
    EXPECT_THROW(ConfigLoader{}.load({}), std::invalid_argument);
}

TEST_F(ConfigLoaderTest, RejectsMissingConfigurationFile) {
    const auto path = temporary_directory / "missing.toml";

    try {
        ConfigLoader{}.load(path);
        FAIL() << "Expected a missing configuration file to be rejected";
    } catch (const std::runtime_error &error) {
        EXPECT_NE(std::string(error.what()).find(
                      "Unable to access configuration file"),
                  std::string::npos)
            << error.what();
    }
}

TEST_F(ConfigLoaderTest, RejectsFilesystemStatusErrors) {
    const auto file = temporary_directory / "not_a_directory";
    std::ofstream(file) << "not a directory";

    try {
        ConfigLoader{}.load(file / "config.toml");
        FAIL() << "Expected a filesystem status error";
    } catch (const std::runtime_error &error) {
        EXPECT_NE(std::string(error.what()).find("Unable to access configuration file"),
                  std::string::npos);
    }
}

TEST_F(ConfigLoaderTest, RejectsDirectories) {
    EXPECT_THROW(ConfigLoader{}.load(temporary_directory), std::runtime_error);
    try {
        ConfigLoader{}.load(temporary_directory);
        FAIL();
    } catch (const std::runtime_error &error) {
        EXPECT_NE(std::string(error.what()).find("not a regular file"),
                  std::string::npos);
    }
}

TEST_F(ConfigLoaderTest, RejectsMalformedToml) {
    expectInvalidConfiguration("[weather\nenabled = true",
                               "Failed to parse config file");
}

TEST_F(ConfigLoaderTest, RejectsNonTableSections) {
    expectInvalidConfiguration("weather = true", "'weather': must be a table");
}

TEST_F(ConfigLoaderTest, RejectsWrongTypesForSettings) {
    expectInvalidConfiguration("[general]\ncache_dir = 42",
                               "general.cache_dir");
    expectInvalidConfiguration("[weather]\nenabled = \"yes\"",
                               "weather.enabled");
    expectInvalidConfiguration("[weather]\ninterval_seconds = \"30\"",
                               "weather.interval_seconds");
    expectInvalidConfiguration("[weather]\nlatitude = \"30\"",
                               "weather.latitude");
    expectInvalidConfiguration("[weather]\nlongitude = \"77\"",
                               "weather.longitude");
    expectInvalidConfiguration("[battery]\nenabled = [true]", "battery.enabled");
    expectInvalidConfiguration("[battery]\ninterval_seconds = [30.0]",
                               "battery.interval_seconds");
    expectInvalidConfiguration("[memory]\nenabled = [true]", "memory.enabled");
    expectInvalidConfiguration("[memory]\ninterval_seconds = [10.0]",
                               "memory.interval_seconds");
    expectInvalidConfiguration("[wallpaper]\nenabled = [true]",
                               "wallpaper.enabled");
    expectInvalidConfiguration("[wallpaper]\ninterval_seconds = [300.0]",
                               "wallpaper.interval_seconds");
    expectInvalidConfiguration("[wallpaper]\ndirectory = 123",
                               "wallpaper.directory");
    expectInvalidConfiguration("[quotes]\nenabled = [true]", "quotes.enabled");
    expectInvalidConfiguration("[quotes]\ninterval_seconds = [3600.0]",
                               "quotes.interval_seconds");
}

TEST_F(ConfigLoaderTest, RejectsNonPositiveIntervals) {
    expectInvalidConfiguration("[weather]\ninterval_seconds = 0",
                               "weather.interval_seconds");
    expectInvalidConfiguration("[battery]\ninterval_seconds = -1",
                               "battery.interval_seconds");
    expectInvalidConfiguration("[memory]\ninterval_seconds = 0",
                               "memory.interval_seconds");
    expectInvalidConfiguration("[wallpaper]\ninterval_seconds = -10",
                               "wallpaper.interval_seconds");
    expectInvalidConfiguration("[quotes]\ninterval_seconds = 0",
                               "quotes.interval_seconds");
}

TEST_F(ConfigLoaderTest, RejectsEmptyPaths) {
    expectInvalidConfiguration("[general]\ncache_dir = \"\"",
                               "general.cache_dir");
    expectInvalidConfiguration("[wallpaper]\ndirectory = \"\"",
                               "wallpaper.directory");
}

TEST_F(ConfigLoaderTest, RejectsCoordinatesOutsideSupportedRanges) {
    expectInvalidConfiguration("[weather]\nlatitude = -90.1",
                               "weather.latitude");
    expectInvalidConfiguration("[weather]\nlatitude = 90.1",
                               "weather.latitude");
    expectInvalidConfiguration("[weather]\nlongitude = -180.1",
                               "weather.longitude");
    expectInvalidConfiguration("[weather]\nlongitude = 180.1",
                               "weather.longitude");
}

TEST_F(ConfigLoaderTest, RejectsNonFiniteCoordinates) {
    expectInvalidConfiguration("[weather]\nlatitude = nan",
                               "weather.latitude");
    expectInvalidConfiguration("[weather]\nlongitude = inf",
                               "weather.longitude");
}

} // namespace
