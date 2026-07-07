#pragma once

#include "IService.hpp"
#include <filesystem>
#include <optional>
#include <random>
#include <vector>
class WallpaperService : public IService {
      private:
        std::filesystem::path wallpaperDirPath;
        std::filesystem::path wallpaperPath;
        std::mt19937 rng;
        std::optional<std::vector<std::filesystem::path>>
        scanWallpaperDirectory();

      public:
        WallpaperService(std::filesystem::path wallpaperDirPath,
                         std::filesystem::path wallpaperPath);
        std::optional<std::filesystem::path> chooseWallpaper();

        bool update() override;
};
