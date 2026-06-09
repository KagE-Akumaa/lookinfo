#include "quoteService.hpp"
#include "fileHandler.hpp"
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <random>
#include <unistd.h>
#include <vector>

QuoteService::QuoteService(std::filesystem::path quote_path) {
        this->quote_path = quote_path;
}
// NOTE: This function is responsible to get a random quote from the quotes.txt
// (make sure the file exist)
std::string QuoteService::getQuote() {
        // Step - 1 Open the file

        // NOTE: Later the file path will be configured from the configuration
        // files
        std::filesystem::path file_path = "../data/quotes.txt";

        // NOTE: permission only required when we are creating the file using
        // O_CREAT in this case it's ignored
        int fd = open(file_path.c_str(), O_RDONLY, 0600);

        if (fd == -1) {
                std::cerr << "Error opening the file" << std::strerror(errno)
                          << std::endl;
                return "";
        }

        // Step - 2 Now read the file

        size_t buffer_size = 8000;
        std::string buffer(buffer_size, '\0');
        std::string quotes_data;

        while (true) {

                ssize_t bytes = read(fd, buffer.data(), buffer_size);

                if (bytes == -1) {
                        // Means we have encountered an error
                        std::cerr << "Error while reading the file"
                                  << std::strerror(errno) << std::endl;
                        // NOTE: -----> REMEMBER to close the fd :)
                        close(fd);
                        return "";
                }

                quotes_data.append(buffer.begin(), buffer.begin() + bytes);
                if (bytes == 0) {
                        // Means we have reached end of file
                        break;
                }
        }
        // NO need for this now
        close(fd);
        // Step - 3 Get this into a vector
        std::vector<std::string> quotes;

        std::string temp = "";
        for (size_t i = 0; i < quotes_data.size(); i++) {
                if (quotes_data[i] == '\n') {
                        quotes.push_back(temp);
                        temp = "";
                        continue;
                }
                temp += quotes_data[i];
        }

        if (!temp.empty()) {
                quotes.push_back(temp);
        }

        // NOTE: Check if quotes is empty
        if (quotes.empty()) {
                return "Check the quotes.txt";
        }

        // Step - 4 pick a random index
        std::random_device rd;

        // 2. Initialize the standard mersenne_twister_engine with the seed
        std::mt19937 gen(rd());

        // 3. Define the inclusive range [min, max]
        std::uniform_int_distribution<int> distrib(0, quotes.size() - 1);

        // 4. Generate the random number
        int random_num = distrib(gen);

        // Step - 5   NOTE: REMEMBER TO CLOSE THE FD
        return quotes[random_num];
}

// NOTE: This function will be fired when scheduler schedule the quotes change
bool QuoteService::update() {
        //
        std::string quote_info = getQuote();

        if (!fileWriter(quote_path, quote_info))
                return false;
        return true;
}
