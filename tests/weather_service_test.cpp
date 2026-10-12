#include "weather.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace {

class WeatherServiceTest : public ::testing::Test {
  protected:
    void SetUp() override {
        const auto unique_id = std::chrono::steady_clock::now()
                                   .time_since_epoch()
                                   .count();
        temporary_directory = std::filesystem::temp_directory_path() /
                              ("lookinfo_weather_test_" +
                               std::to_string(unique_id));
        std::filesystem::create_directories(temporary_directory);
        response_file = temporary_directory / "weather.json";
        output_file = temporary_directory / "weather.txt";
    }

    void TearDown() override {
        std::filesystem::remove_all(temporary_directory);
    }

    std::string responseUrl() const {
        return "file://" + response_file.string();
    }

    void writeResponse(const std::string &response) {
        std::ofstream file(response_file);
        ASSERT_TRUE(file.is_open());
        file << response;
    }

    std::filesystem::path temporary_directory;
    std::filesystem::path response_file;
    std::filesystem::path output_file;
};

TEST_F(WeatherServiceTest, ReadsWeatherValuesFromResponse) {
    writeResponse(R"json({
        "current": {"temperature_2m": 18.25},
        "daily": {
            "temperature_2m_max": [27.5],
            "temperature_2m_min": [9.75]
        }
    })json");

    WeatherService service(output_file, responseUrl());

    const Weather_info weather = service.getWeatherInfo();

    EXPECT_DOUBLE_EQ(weather.current, 18.25);
    EXPECT_DOUBLE_EQ(weather.max, 27.5);
    EXPECT_DOUBLE_EQ(weather.min, 9.75);
}

TEST_F(WeatherServiceTest, UpdateWritesFormattedWeatherInformation) {
    writeResponse(R"json({
        "current": {"temperature_2m": 18.25},
        "daily": {
            "temperature_2m_max": [27.5],
            "temperature_2m_min": [9.75]
        }
    })json");

    WeatherService service(output_file, responseUrl());

    ASSERT_TRUE(service.update());

    std::ifstream output(output_file);
    const std::string contents((std::istreambuf_iterator<char>(output)),
                               std::istreambuf_iterator<char>());

    EXPECT_EQ(contents, "🌦️ 18.2°C | ↑27.5° ↓9.8°");
}

TEST_F(WeatherServiceTest, ReturnsZeroValuesWhenResponseCannotBeRead) {
    WeatherService service(output_file, responseUrl());

    const Weather_info weather = service.getWeatherInfo();

    EXPECT_DOUBLE_EQ(weather.current, 0.0);
    EXPECT_DOUBLE_EQ(weather.max, 0.0);
    EXPECT_DOUBLE_EQ(weather.min, 0.0);
}

} // namespace
