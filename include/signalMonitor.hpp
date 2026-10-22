#pragma once

#include <expected>
#include <system_error>

struct SignalMonitorError {
        std::error_code creationError;
        std::error_code restoreError{}; // Zero means no restoration error
};

class SignalMonitor {

        int signalFd{-1};

      public:
        explicit SignalMonitor(int fd) noexcept;
        ~SignalMonitor();

        SignalMonitor(const SignalMonitor &other) = delete;
        SignalMonitor &operator=(const SignalMonitor &other) = delete;

        SignalMonitor(SignalMonitor &&other) noexcept;
        SignalMonitor &operator=(SignalMonitor &&other) noexcept;

        [[nodiscard]] static std::expected<SignalMonitor, SignalMonitorError>
        getSignalFd() noexcept;

        [[nodiscard]] int getFd() noexcept;
};
