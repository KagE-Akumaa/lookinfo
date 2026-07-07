#pragma once

#include "IService.hpp"
#include <filesystem>
#include <optional>
#include <string>
#include <vector>
class WallpaperService : public IService {
      private:
        std::string wallpaperDirPath;
        std::string wallpaperPath;
        std::optional<std::vector<std::filesystem::path>>
        scanWallpaperDirectory();
        

      public:
        WallpaperService(std::filesystem::path wallpaperDirPath,
                         std::filesystem::path wallpaperPath);
        std::optional<std::filesystem::path> getWallpaperInfo();

        bool update() override;
};
