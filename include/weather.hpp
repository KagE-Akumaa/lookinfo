#pragma once

#include <curl/curl.h>
#include <memory>

struct Weather_info {
        double current;
        double max;
        double min;
};
class Weather {
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

      public:
        Weather();

        Weather(const Weather &) = delete;

        Weather &operator=(const Weather &) = delete;

        Weather_info getWeatherInfo(std::string url);
};
