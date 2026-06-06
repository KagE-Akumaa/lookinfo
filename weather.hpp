#pragma once

#include <string>

class Weather {

        std::string city;
        std::string cmd;

        std::string generateCommand(const std::string &city);

      public:
        // NOTE: Constructor will take the city if given and creates the cmd
        Weather(std::string city = "Shimla");
        std::string exec();
};
