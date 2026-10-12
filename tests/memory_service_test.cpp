#include "memoryService.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace {

class MemoryServiceTest : public ::testing::Test {
  protected:
    void SetUp() override {
        const auto unique_id = std::chrono::steady_clock::now()
                                   .time_since_epoch()
                                   .count();
        temporary_directory = std::filesystem::temp_directory_path() /
                              ("lookinfo_memory_test_" +
                               std::to_string(unique_id));
        std::filesystem::create_directories(temporary_directory);
        memory_source_file = temporary_directory / "meminfo";
        output_file = temporary_directory / "memory.txt";
    }

    void TearDown() override {
        std::filesystem::remove_all(temporary_directory);
    }

    void writeMemoryInfo(const std::string &contents) {
        std::ofstream file(memory_source_file);
        ASSERT_TRUE(file.is_open());
        file << contents;
    }

    std::filesystem::path temporary_directory;
    std::filesystem::path memory_source_file;
    std::filesystem::path output_file;
};

TEST_F(MemoryServiceTest, ReadsTotalAndAvailableMemory) {
    writeMemoryInfo("MemTotal:       8388608 kB\n"
                    "MemAvailable:   4194304 kB\n");

    MemoryService service(output_file, memory_source_file);

    const auto memory_info = service.getMemoryInfo();

    ASSERT_TRUE(memory_info.has_value());
    EXPECT_EQ(memory_info->mem_total_kb, 8388608);
    EXPECT_EQ(memory_info->mem_available_kb, 4194304);
}

TEST_F(MemoryServiceTest, ReturnsNoValueWhenMemoryFileIsMissing) {
    MemoryService service(output_file, memory_source_file);

    EXPECT_FALSE(service.getMemoryInfo().has_value());
}

TEST_F(MemoryServiceTest, UpdateWritesFormattedMemoryInformation) {
    writeMemoryInfo("MemTotal:       8388608 kB\n"
                    "MemAvailable:   4194304 kB\n");

    MemoryService service(output_file, memory_source_file);

    ASSERT_TRUE(service.update());

    std::ifstream output(output_file);
    const std::string contents((std::istreambuf_iterator<char>(output)),
                               std::istreambuf_iterator<char>());

    EXPECT_EQ(contents, "🧠 4.0G / 8.0G");
}

} // namespace
