#include "wallpaper.hpp"
#include "fileHandler.hpp"
#include <filesystem>
#include <iostream>
#include <optional>
#include <random>

WallpaperService::WallpaperService(std::filesystem::path wallpaperDirPath,
                                   std::filesystem::path wallpaperPath)
    : wallpaperDirPath(wallpaperDirPath), wallpaperPath(wallpaperPath) {}

std::optional<std::vector<std::filesystem::path>>
WallpaperService::scanWallpaperDirectory() {
        // Step 1 To check if the wallpaper directory exist
        if (!std::filesystem::exists(wallpaperDirPath) ||
            !std::filesystem::is_directory(wallpaperDirPath)) {
                std::cerr << "Wallpaper directory does not exist or is not a "
                             "directory: "
                          << wallpaperDirPath << '\n';
                return std::nullopt;
        }
        // Step 2 read the wallpaper directory using directory_iterator (later
        // recursive directory_iterator)
        std::vector<std::filesystem::path> ver;
        for (auto const &entry :
             std::filesystem::directory_iterator(wallpaperDirPath)) {
                if (entry.path().extension() == ".jpg" ||
                    entry.path().extension() == ".png" ||
                    entry.path().extension() == ".jpeg") {
                        ver.push_back(entry.path());
                }
        }
        return ver;
}

std::optional<std::filesystem::path> WallpaperService::getWallpaperInfo() {
        //
        auto wallpapers = scanWallpaperDirectory();

        if (wallpapers == std::nullopt) {
                std::cerr << "Error opening Wallpaper Directory" << std::endl;
                return std::nullopt;
        }

        // check if the wallpaper vector is empty no wallpaper's in the
        // directory
        if (wallpapers->empty()) {
                std::cerr << "No wallpaper found\n";
                return std::nullopt;
        }
        // 1. Initialize seed source
        std::random_device rd;

        // 2. Initialize the random engine with the seed
        std::mt19937 gen(rd());

        // 3. Define the distribution range: [0, size - 1] inclusive
        std::uniform_int_distribution<size_t> distr(0, wallpapers->size() - 1);

        // 4. Generate the random index
        size_t random_index = distr(gen);

        return (*wallpapers)[random_index];
}

bool WallpaperService::update() {
        //
        auto wallpaperInfo = getWallpaperInfo();

        if (wallpaperInfo == std::nullopt) {
                std::cerr << "Error obtaining wallpaper information"
                          << std::endl;
                return false;
        }

        std::string text = wallpaperInfo->c_str();
        if (!fileWriter(wallpaperPath, text)) {
                return false;
        }

        return true;
}
