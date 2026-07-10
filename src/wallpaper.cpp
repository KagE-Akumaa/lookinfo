#include "wallpaper.hpp"
#include "fileHandler.hpp"
#include <filesystem>
#include <iostream>
#include <optional>
#include <random>

WallpaperService::WallpaperService(std::filesystem::path wallpaperDirPath,
                                   std::filesystem::path wallpaperPath)
    : wallpaperDirPath(std::move(wallpaperDirPath)),
      wallpaperPath(std::move(wallpaperPath)), rng(std::random_device{}()) {}

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
        // skip the files for which we don't have permission
        std::vector<std::filesystem::path> ver;
        try {
                for (auto const &entry :
                     std::filesystem::recursive_directory_iterator(
                         wallpaperDirPath, std::filesystem::directory_options::
                                               skip_permission_denied)) {
                        if (!entry.is_regular_file()) {
                                continue;
                        }
                        auto ext = entry.path().extension();
                        if (ext == ".jpg" || ext == ".png" || ext == ".jpeg") {
                                ver.push_back(entry.path());
                        }
                }
        } catch (const std::filesystem::filesystem_error &e) {
                std::cerr << e.what() << std::endl;
                return std::nullopt;
        }
        return ver;
}

std::optional<std::filesystem::path> WallpaperService::chooseWallpaper() {
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
        // 3. Define the distribution range: [0, size - 1] inclusive
        std::uniform_int_distribution<size_t> distr(0, wallpapers->size() - 1);

        // 4. Generate the random index
        size_t random_index = distr(rng);

        return (*wallpapers)[random_index];
}

bool WallpaperService::update() {
        //
        auto wallpaper = chooseWallpaper();

        if (wallpaper == std::nullopt) {
                std::cerr << "Error obtaining wallpaper information"
                          << std::endl;
                return false;
        }

        while (wallpaper.value() == lastWallpaper) {
                wallpaper = chooseWallpaper();
                if (wallpaper == std::nullopt) {
                        std::cerr << "Error obtaining wallpaper information"
                                  << std::endl;
                        return false;
                }
        }

        std::string text = wallpaper->string();
        if (!fileWriter(wallpaperPath, text)) {
                return false;
        }
        lastWallpaper = wallpaper.value();

        return true;
}
