#include "weather.hpp"
#include <array>
#include <cstring>
#include <iostream>

std::string Weather::generateCommand(const std::string &city) {
        //
        std::string curl{"curl"};
        std::string info = "wttr.in/" + city + "?format=%c+%t";
        std::string cmd = curl + " " + info;

        return cmd;
}
Weather::Weather(std::string city) {
        this->city = city;

        this->cmd = generateCommand(city);
}
// NOTE: This function will not throw it will return an empty string on faliure
std::string Weather::exec() {
        std::array<char, 128> buffer{};
        std::string result;

        FILE *pipe = popen(cmd.c_str(), "r");

        if (!pipe) {
                std::cerr << "Error executing the command"
                          << std::strerror(errno) << std::endl;
                return "";
        }

        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
                result += buffer.data();
        }

        while (!result.empty() && result.back() == '\n') {
                result.pop_back();
        }
        pclose(pipe);

        return result;
}
