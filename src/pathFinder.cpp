#include "pathFinder.hpp"
#include <iostream>
#include <optional>

// NOTE: This function will give the path to the .cache directory
std::optional<std::filesystem::path> getCacheDirectory() {
        //
        // TODO: - create a directory if not present in ~/.cache lookinfo

        // NOTE: getenv() return the val or nullptr if it does not exist
        char *h = std::getenv("HOME");

        if (h == nullptr) {
                std::cout << "HOME DOES NOT EXIST" << std::endl;
                return std::nullopt;
        }

        std::filesystem::path home_path = h;

        // NOTE: To concatenate these to make this relative
        // std::filesystem::path d_path = ".cache/lookinfo";

        // NOTE: To concatenate filesystem::path we can use '/' this
        std::filesystem::path final_path = home_path / ".cache" / "lookinfo";

        // NOTE: Now we need to create the directory if it does not exist

        if (!std::filesystem::exists(final_path)) {
                try {
                        if (std::filesystem::create_directories(final_path)) {
                                std::cout << "Directory created" << std::endl;
                        }
                } catch (std::filesystem::filesystem_error &e) {
                        std::cerr << "Error " << e.what() << std::endl;
                        return std::nullopt;
                }
        }

        // NOTE: At this point we do have the lookinfo directory in .cache now
        // we need to create the files for weather, quote, time
        return final_path;
}
