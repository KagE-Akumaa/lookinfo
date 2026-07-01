#pragma once

#include "IService.hpp"
#include <filesystem>
#include <string>
class QuoteService : IService {

      private:
        std::filesystem::path quote_path;

      public:
        QuoteService(std::filesystem::path quote_path);
        std::string getQuote();
        bool update() override;
};
