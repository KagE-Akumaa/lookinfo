#include <chrono>
#include <curl/curl.h>
#include <iostream>
#include <string>

// Callback function to handle incoming data chunks
size_t callback(void *contents, size_t size, size_t nmemb, void *userp) {
        size_t totalSize = size * nmemb;
        std::string *responseString = static_cast<std::string *>(userp);
        responseString->append(static_cast<char *>(contents), totalSize);
        return totalSize;
}
int main() {
        auto start = std::chrono::steady_clock::now();
        // Initialize curl -> It's a one time operation
        curl_global_init(CURL_GLOBAL_DEFAULT);

        // Got the handle object
        CURL *handle = curl_easy_init();

        std::string url =
            "https://api.open-meteo.com/v1/"
            "forecast?latitude=31.1044&longitude=77.1666&models=best_match&"
            "current=temperature_2m&timezone=auto&forecast_days=1";
        if (handle) {

                std::string response_body;
                long http_code;

                curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
                curl_easy_setopt(handle, CURLOPT_HTTP_VERSION,
                                 CURL_HTTP_VERSION_1_1);
                // Step -1 Set the url -> requires handle, flag, the url
                curl_easy_setopt(handle, CURLOPT_URL, url.c_str());

                // Step - 2 Pass the call back function to handle data transfers
                // - > handle, writefunction flag, the callback function returns
                // the bytes
                curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, callback);

                // Step - 3 Pass the pointer to our string where data will be
                // stored
                curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response_body);

                curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);

                curl_easy_setopt(handle, CURLOPT_FORBID_REUSE, 1L);

                // Step - 4 perform the operation synchronously
                CURLcode res = curl_easy_perform(handle);

                // 5. Check for network errors and retrieve the HTTP status code
                if (res == CURLE_OK) {
                        curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE,
                                          &http_code);

                        std::cout << "HTTP Status Code: " << http_code
                                  << std::endl;
                        std::cout << "Response Body: \n"
                                  << response_body << std::endl;
                } else {
                        std::cerr << "curl_easy_perform() failed: "
                                  << curl_easy_strerror(res) << std::endl;
                }
                std::cout << "Before cleanup\n";
                // 5. Cleanup the handle
                curl_easy_cleanup(handle);

                std::cout << "After cleanup\n";
        }
        auto end = std::chrono::steady_clock::now();

        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // 4. Output the result using .count()
        std::cout << "Elapsed time: " << elapsed.count() << " ms\n";
        std::cout << "before Global cleanup\n";
        // Global cleanup call once only

        curl_global_cleanup();

        std::cout << "after Global cleanup\n";
        return 0;
}
