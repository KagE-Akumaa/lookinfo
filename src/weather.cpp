#include "weather.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

Weather::Weather() {
        static CurlGlobal globalSetup;
        curl.reset(curl_easy_init());
}
// Callback function to handle incoming data chunks
size_t callback(void *contents, size_t size, size_t nmemb, void *userp) {
        size_t totalSize = size * nmemb;
        std::string *responseString = static_cast<std::string *>(userp);
        responseString->append(static_cast<char *>(contents), totalSize);
        return totalSize;
}

Weather_info Weather::getWeatherInfo(std::string url) {
        std::string response_body;
        Weather_info w{};
        if (curl) {

                long http_code;

                // Step -1 Set the url -> requires handle, flag, the url
                curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());

                // Step - 2 Pass the call back function to handle data transfers
                // - > handle, writefunction flag, the callback function returns
                // the bytes
                curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, callback);

                // Step - 3 Pass the pointer to our string where data will be
                // stored
                curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response_body);

                // Step - 4 perform the operation synchronously
                CURLcode res = curl_easy_perform(curl.get());

                // 5. Check for network errors and retrieve the HTTP status code
                if (res == CURLE_OK) {
                        curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE,
                                          &http_code);

                        std::cout << "HTTP Status Code: " << http_code
                                  << std::endl;
                        std::cout << "Response Body: \n"
                                  << response_body << std::endl;
                } else {
                        std::cerr << "curl_easy_perform() failed: "
                                  << curl_easy_strerror(res) << std::endl;

                        return w;
                }

                if (response_body.empty()) {
                        std::cerr << "Response body is empty" << std::endl;
                        return w;
                }

                json data = json::parse(response_body);

                w.current = data["current"]["temperature_2m"];
                w.max = data["daily"]["temperature_2m_max"][0];
                w.min = data["daily"]["temperature_2m_min"][0];
        }

        return w;
}
