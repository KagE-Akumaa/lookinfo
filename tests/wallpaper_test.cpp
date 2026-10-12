#include "wallpaper.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

class WallpaperServiceTest : public ::testing::Test {
  protected:
    void SetUp() override {
        const auto unique_id = std::chrono::steady_clock::now()
                                   .time_since_epoch()
                                   .count();
        wallpaper_directory = std::filesystem::temp_directory_path() /
                              ("lookinfo_wallpaper_test_" +
                               std::to_string(unique_id));
        std::filesystem::create_directories(wallpaper_directory / "nested");
        output_file = wallpaper_directory / "selected_wallpaper.txt";
    }

    void TearDown() override {
        std::filesystem::remove_all(wallpaper_directory);
    }

    void createFile(const std::filesystem::path &path) {
        std::ofstream file(path);
        ASSERT_TRUE(file.is_open());
    }

    std::filesystem::path wallpaper_directory;
    std::filesystem::path output_file;
};

TEST_F(WallpaperServiceTest, ChoosesSupportedWallpaperRecursively) {
    const auto jpg = wallpaper_directory / "wallpaper.jpg";
    const auto png = wallpaper_directory / "nested" / "wallpaper.png";
    const auto jpeg = wallpaper_directory / "wallpaper.jpeg";
    const auto ignored = wallpaper_directory / "notes.txt";

    createFile(jpg);
    createFile(png);
    createFile(jpeg);
    createFile(ignored);

    WallpaperService service(wallpaper_directory, output_file);

    for (int attempt = 0; attempt < 20; ++attempt) {
        const auto selected = service.chooseWallpaper();

        ASSERT_TRUE(selected.has_value());
        EXPECT_TRUE(selected == jpg || selected == png || selected == jpeg);
    }
}

TEST_F(WallpaperServiceTest, ReturnsEmptyWhenDirectoryDoesNotExist) {
    const auto missing_directory = wallpaper_directory / "missing";
    WallpaperService service(missing_directory, output_file);

    EXPECT_FALSE(service.chooseWallpaper().has_value());
}

TEST_F(WallpaperServiceTest, UpdateWritesSelectedWallpaperPath) {
    const auto wallpaper = wallpaper_directory / "wallpaper.png";
    createFile(wallpaper);

    WallpaperService service(wallpaper_directory, output_file);

    ASSERT_TRUE(service.update());
    ASSERT_TRUE(std::filesystem::exists(output_file));

    std::ifstream output(output_file);
    const std::string selected_path((std::istreambuf_iterator<char>(output)),
                                    std::istreambuf_iterator<char>());

    EXPECT_EQ(selected_path, wallpaper.string());
}

} // namespace
