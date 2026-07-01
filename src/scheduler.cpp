#include "scheduler.hpp"
#include <thread>

bool Scheduler::Compare::operator()(const Scheduler::ScheduledTask &lhs,
                                    const Scheduler::ScheduledTask &rhs) const {
        // TODO:
        // Return true if lhs should have LOWER priority than rhs.
        return lhs.nextRun > rhs.nextRun;
}

void Scheduler::add(IService *service, std::chrono::seconds interval) {
        ScheduledTask task;

        task.service = service;
        task.interval = interval;
        task.nextRun = std::chrono::steady_clock::now() + interval;

        tasks.push(task);
}

void Scheduler::run() {
        while (true) {
                // TODO:
                // 1. Get the next task.
                // 2. Remove it from the queue.
                // 3. Sleep until its execution time.
                // 4. Execute service->update().
                // 5. Compute the next execution time.
                // 6. Push it back into the queue.
                auto nextTask = tasks.top();
                tasks.pop();

                std::this_thread::sleep_until(nextTask.nextRun);
                nextTask.service->update();

                nextTask.nextRun += nextTask.interval;

                tasks.push(nextTask);
        }
}
