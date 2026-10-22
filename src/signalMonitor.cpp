#include "signalMonitor.hpp"

#include <csignal>
#include <sys/signalfd.h>
#include <unistd.h>

// CONSTRUCTOR
SignalMonitor::SignalMonitor(int fd) noexcept : signalFd(fd) {}

// MOVE CONSTRUCTOR
SignalMonitor::SignalMonitor(SignalMonitor &&other) noexcept
    : signalFd(other.signalFd) {
        other.signalFd = -1;
}
// MOVE ASSIGNMENT OPERATOR
SignalMonitor &SignalMonitor::operator=(SignalMonitor &&other) noexcept {
        if (this == &other) {
                return *this;
        }

        if (signalFd != -1) {
                close(signalFd);
        }

        signalFd = other.signalFd;
        other.signalFd = -1;
        return *this;
}
std::expected<SignalMonitor, SignalMonitorError>
SignalMonitor::getSignalFd() noexcept {
        // Step 1 Define the Signals which we want to recieve and block their
        // standard propagation

        sigset_t mask{};
        sigset_t oldMask{};
        sigemptyset(&mask);
        sigaddset(&mask, SIGINT);
        sigaddset(&mask, SIGTERM);

        // 2. Block them so they can be consumed through a signalfd
        if (sigprocmask(SIG_BLOCK, &mask, &oldMask) == -1) {

                const std::error_code error{errno, std::generic_category()};

                return std::unexpected(SignalMonitorError{error, {}});
        }

        // 3. Create fd for these signals
        int sigFd = signalfd(-1, &mask, SFD_CLOEXEC | SFD_NONBLOCK);

        if (sigFd == -1) {
                const std::error_code creationError{errno,
                                                    std::generic_category()};
                // Restore oldMask
                if (sigprocmask(SIG_SETMASK, &oldMask, nullptr) == -1) {
                        const std::error_code restoreError{
                            errno, std::generic_category()};

                        return std::unexpected(
                            SignalMonitorError{creationError, restoreError});
                }

                return std::unexpected(SignalMonitorError{creationError, {}});
        }

        return SignalMonitor(sigFd);
}
int SignalMonitor::getFd() noexcept { return signalFd; }
// DESTRUCTOR
SignalMonitor::~SignalMonitor() {
        if (signalFd != -1) {
                close(signalFd);
        }
}
