#include "batteryService.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

class BatteryServiceTest : public ::testing::Test {
  protected:
    void SetUp() override {
        const auto unique_id = std::chrono::steady_clock::now()
                                   .time_since_epoch()
                                   .count();
        temporary_directory = std::filesystem::temp_directory_path() /
                              ("lookinfo_battery_test_" +
                               std::to_string(unique_id));
        battery_directory = temporary_directory / "BAT0";
        output_file = temporary_directory / "battery.txt";
        std::filesystem::create_directories(battery_directory);
    }

    void TearDown() override {
        std::filesystem::remove_all(temporary_directory);
    }

    void writeFile(const std::filesystem::path &path,
                   const std::string &contents) {
        std::ofstream file(path);
        ASSERT_TRUE(file.is_open());
        file << contents;
    }

    std::filesystem::path temporary_directory;
    std::filesystem::path battery_directory;
    std::filesystem::path output_file;
};

TEST_F(BatteryServiceTest, ReadsBatteryInformationFromBatteryDirectory) {
    writeFile(battery_directory / "type", "Battery\n");
    writeFile(battery_directory / "uevent",
              "POWER_SUPPLY_STATUS=Charging\n"
              "POWER_SUPPLY_CAPACITY=87\n");

    BatteryService service(output_file, temporary_directory);

    const auto battery_info = service.getBatteryInfo();

    ASSERT_TRUE(battery_info.has_value());
    EXPECT_EQ(battery_info->status, "Charging");
    EXPECT_EQ(battery_info->percentage, 87);
}

TEST_F(BatteryServiceTest, ReturnsEmptyInformationWhenNoBatteryIsPresent) {
    writeFile(battery_directory / "type", "Mains\n");

    BatteryService service(output_file, temporary_directory);

    const auto battery_info = service.getBatteryInfo();

    ASSERT_TRUE(battery_info.has_value());
    EXPECT_TRUE(battery_info->empty());
}

TEST_F(BatteryServiceTest, UpdateWritesFormattedBatteryInformation) {
    writeFile(battery_directory / "type", "  Battery  \n");
    writeFile(battery_directory / "uevent",
              "POWER_SUPPLY_STATUS=Discharging\n"
              "POWER_SUPPLY_CAPACITY=42\n");

    BatteryService service(output_file, temporary_directory);

    ASSERT_TRUE(service.update());

    std::ifstream output(output_file);
    const std::string contents((std::istreambuf_iterator<char>(output)),
                               std::istreambuf_iterator<char>());

    EXPECT_EQ(contents, "🔋 42% Discharging");
}

} // namespace
