#pragma once

#include "config.hpp"
#include <filesystem>

class ConfigLoader {
      public:
        LookInfoConfig load(const std::filesystem::path &configPath);
};
