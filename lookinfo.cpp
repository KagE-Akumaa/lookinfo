#include "weather.hpp"
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <random>
#include <unistd.h>
#include <vector>

// NOTE: This function is responsible to get a random quote from the quotes.txt
// (make sure the file exist)
std::string getQuotesInfo() {
        // Step - 1 Open the file

        std::filesystem::path file_path = "quotes.txt";

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

// NOTE: This function is responsible to get the current time in a correct
// format (09:05)
std::string generateTime(int hour, int min) {
        //
        // if hour < 10 and min < 10 we can add the 0 in front

        std::string h, m;
        if (hour < 10) {
                h = "0" + std::to_string(hour);
        } else {
                h = std::to_string(hour);
        }
        if (min < 10)
                m = "0" + std::to_string(min);
        else
                m = std::to_string(min);

        std::string final = h + ":" + m;

        return final;
}
// NOTE: This function is responsible for opening the file, writing the data to
// it and closing the file descriptor and to report any errors during the
// execution
int fileHandler(const std::filesystem::path &file_path,
                const std::string &data) {
        // O_CREAT - if not present create one, O_WRONLY - Only for writing,
        // O_TRUNC - if file exist and is opened successfully truncate it's
        // length to 0 0600 is the permission for this file
        int fd = open(file_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0600);

        if (fd == -1) {
                std::cerr << "Error opening the file " << file_path
                          << std::strerror(errno) << std::endl;
                return -1;
        }

        ssize_t bytes = write(fd, data.c_str(), data.size());

        if (bytes == -1) {
                std::cerr << " Failed to write to file" << std::strerror(errno)
                          << std::endl;
                close(fd);
                return -1;
        }
        // NOTE: Remeber to close the file descriptor
        close(fd);

        return 0;
}

int main() {
        std::cout << "HELLO LOOKINFO" << std::endl;
        //
        // TODO: - create a directory if not present in ~/.cache lookinfo

        // NOTE: getenv() return the val or nullptr if it does not exist
        char *h = std::getenv("HOME");

        if (h == nullptr) {
                std::cout << "HOME DOES NOT EXIST" << std::endl;
                return -1;
        }

        std::filesystem::path home_path = h;

        // NOTE: To concatenate these to make this relative
        // std::filesystem::path d_path = ".cache/lookinfo";

        // NOTE: To concatenate filesystem::path we can use '/' this
        std::filesystem::path final_path = home_path / ".cache" / "lookinfo";

        std::cout << final_path << std::endl;

        // NOTE: Now we need to create the directory if it does not exist

        // try {
        //         // NOTE: Creates the directory in the current scope
        //         if (std::filesystem::create_directory(d_path)) {
        //                 // NOTE: Returns true if it created a directory false
        //                 if
        //                 // already exist also on error
        //                 std::cout << "directory created successfully"
        //                           << std::endl;
        //         }
        // } catch (std::filesystem::filesystem_error &e) {
        //         std::cerr << "Error " << e.what() << std::endl;
        //         return -1;
        // }

        if (std::filesystem::exists(final_path)) {
                std::cout << "Directory already exist" << std::endl;
        } else {
                try {
                        if (std::filesystem::create_directories(final_path)) {
                                std::cout << "Directory created" << std::endl;
                        }

                        // NOTE: Can remove this as we check using the
                        // filesystem::exists() else {
                        //         std::cout << "Directory already exist"
                        //                   << std::endl;
                        // }
                } catch (std::filesystem::filesystem_error &e) {
                        std::cerr << "Error " << e.what() << std::endl;
                }
        }
        // NOTE: At this point we do have the lookinfo directory in .cache now
        // we need to create the files for weather, quote, time

        // NOTE: This is for the weather.txt
        std::filesystem::path weather_path = final_path / "weather.txt";
        Weather weather("Ontario");

        std::string weather_info = weather.exec();
        if (fileHandler(weather_path, weather_info) == -1)
                return -1;

        // NOTE: This is for the quotes.txt
        std::filesystem::path quotes_path = final_path / "quotes.txt";
        // TODO: Now we have to get a random quote from the quotes.txt which
        // contains 100 quotes and pass that to fileHandler

        // NOTE: the steps are simle first we read the file into a vector then
        // pick a random one from there

        std::string quotes_info = getQuotesInfo();
        if (fileHandler(quotes_path, quotes_info) == -1) {
                return -1;
        }

        // NOTE: This is for the time.txt
        std::filesystem::path time_path = final_path / "time.txt";

        time_t timestamp;
        time(&timestamp);

        tm *local = localtime(&timestamp);

        if (local == nullptr) {
                std::cerr << "Error getting the current time" << std::endl;
                return -1;
        }
        // NOW we have access to fields like local->tm_hour, tm_min, tm_sec

        // NOTE: The challenge is now to make a zero padding so that when the
        // time is 9:5 it should do 09:05
        std::string time_info = generateTime(local->tm_hour, local->tm_min);
        if (fileHandler(time_path, time_info) == -1)
                return -1;

        return 0;
}
