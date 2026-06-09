#pragma once

#include "IService.hpp"
#include <curl/curl.h>
#include <filesystem>
#include <memory>

struct Weather_info {
        double current;
        double max;
        double min;
};
class WeatherService : public IService {
        //
      private:
        // NOTE: This is a tiny RAII wrapper for Global init/closeup
        struct CurlGlobal {
                CurlGlobal() { curl_global_init(CURL_GLOBAL_DEFAULT); }
                ~CurlGlobal() { curl_global_cleanup(); }
        };

        // NOTE: This is a RAII warpper of easy handle cleanup
        struct CurlDeleter {
                void operator()(CURL *curl) {
                        if (curl)
                                curl_easy_cleanup(curl);
                }
        };

        using SafeCurl = std::unique_ptr<CURL, CurlDeleter>;

        SafeCurl curl;

        std::string url;

        std::filesystem::path weather_path;

      public:
        WeatherService(std::filesystem::path weather_path, std::string url);

        WeatherService(const WeatherService &) = delete;

        WeatherService &operator=(const WeatherService &) = delete;

        Weather_info getWeatherInfo();

        bool update() override;
};
