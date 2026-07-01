#pragma once

#include "IService.hpp"

#include <chrono>
#include <queue>
#include <vector>

class Scheduler {
      private:
        struct ScheduledTask {
                IService *service;

                std::chrono::seconds interval;

                std::chrono::steady_clock::time_point nextRun;
        };

        struct Compare {
                bool operator()(const ScheduledTask &lhs,
                                const ScheduledTask &rhs) const;
        };

        std::priority_queue<ScheduledTask, std::vector<ScheduledTask>, Compare>
            tasks;

      public:
        Scheduler() = default;

        void add(IService *service, std::chrono::seconds interval);

        void run();
};
;
