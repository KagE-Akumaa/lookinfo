#include "batteryService.hpp"
#include "memoryService.hpp"
#include "pathFinder.hpp"
#include "quoteService.hpp"
#include "weather.hpp"
#include <iostream>

int main() {

        // Step - 1 Get the cache directory path

        auto dirPath = getCacheDirectory();

        if (!dirPath) {
                std::cerr << "Failed to get cache directory\n";
                return -1;
        }

        std::cout << *dirPath << std::endl;

        // Step - 2 Weather path , quotes path

        auto weather_path = *dirPath / "weather.txt";
        auto quotes_path = *dirPath / "quotes.txt";
        auto battery_path = *dirPath / "battery.txt";
        auto memory_path = *dirPath / "memory.txt";

        // Step - 3 Initialize the weather and quotes module

        // For weather get the api url
        std::string weather_api_url =
            "https://api.open-meteo.com/v1/"
            "forecast?latitude=31.1044&longitude=77.1666&daily="
            "temperature_2m_max,temperature_2m_min&current="
            "temperature_2m&timezone=auto&forecast_days=1";

        WeatherService weather(weather_path, weather_api_url);

        QuoteService quotes(quotes_path);

        BatteryService battery(battery_path);

        MemoryService memory(memory_path);

        // Step - 4 Call the update function to run the WeatherService and
        // QuoteService

        if (!weather.update()) {
                std::cerr << "Failed to update weather" << std::endl;
        }
        if (!quotes.update()) {
                std::cerr << "Failed to update quotes" << std::endl;
        }

        if (!battery.update()) {
                std::cerr << "Failed to update battery" << std::endl;
        }

        if (!memory.update()) {
                std::cerr << "Failed to update memory" << std::endl;
        }

        return 0;
}
