# LookInfo Production Readiness Plan

This document is the implementation checklist for turning LookInfo from a development prototype into a reliable standalone Linux application.

The recommended target is a Linux user service that:

- Runs continuously in the background.
- Collects weather, memory, battery, wallpaper, and quote data.
- Writes cache files atomically under the user's cache directory.
- Loads runtime settings from a configuration file.
- Shuts down cleanly when stopped by the user or operating system.
- Can be built, tested, installed, packaged, and released reproducibly.

The project currently builds with CMake, uses C++20, links to libcurl, and has GoogleTest coverage for the wallpaper, battery, weather, and memory services.

## Definition of done

The project is production-ready when:

- A clean checkout builds without developer-specific paths.
- No runtime path is hard-coded to `/home/akumaa`.
- Runtime settings can be changed without recompiling.
- SIGINT and SIGTERM stop the process cleanly.
- Network, JSON, filesystem, and parsing failures do not crash the process.
- Cache writes are atomic and parent directories are created automatically.
- All services have success and failure tests.
- Tests run in CI on GCC and Clang.
- The application has install rules and a user-level systemd service.
- A release artifact can be created from a version tag.
- A new user can install and run the application using the README.

## Current baseline

Run this before starting a phase and after every major change:

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build -j2
ctest --test-dir build --output-on-failure
```

At the time this document was created, the suite contained 12 tests.

Record a clean baseline before large changes:

```bash
git status --short
git add .
git commit -m "Establish production readiness baseline"
```

Do not commit generated build directories unless the project explicitly decides to distribute them.

## Phase 1: Establish structure and standards

### 1.1 Organize the repository

Use a structure like:

```text
.
├── CMakeLists.txt
├── cmake/
├── config/
│   └── lookinfo.example.toml
├── data/
│   └── quotes.txt
├── include/
├── packaging/
│   └── systemd/
├── src/
├── tests/
├── README.md
└── PRODUCTION_READINESS.md
```

Keep `lockinfo_core` as the reusable library. Keep `main.cpp` limited to startup, configuration, service construction, signal handling, and scheduler wiring.

### 1.2 Add coding checks

Add a `.clang-format` file and format all C++ files consistently. Add optional CMake targets for:

- clang-format checks
- clang-tidy
- compiler warnings
- static analysis

Enable warnings in all builds. Enable `-Werror` in CI after existing warnings are resolved.

### 1.3 Define supported platforms

Initially document Linux as the supported platform. The code uses `/proc`, `/sys`, POSIX file APIs, and Linux filesystem behavior, so do not claim Windows or macOS support yet.

### Acceptance criteria

- A clean checkout has no developer-specific paths.
- Formatting is consistent.
- Supported OS and dependencies are documented.
- The existing build and tests still pass.

## Phase 2: Add configuration support

The current application hard-codes weather coordinates, wallpaper paths, intervals, and other runtime values in `src/main.cpp`. Move these values into a configuration object.

### 2.1 Create a configuration model

Add a configuration type containing:

- Cache directory
- Wallpaper directory
- Quote file
- Latitude and longitude
- Weather interval
- Memory interval
- Battery interval
- Wallpaper interval
- Quote interval

Keep loading and validation separate from service implementations.

### 2.2 Define configuration lookup

Use this lookup order:

1. A path supplied by a command-line option.
2. `$XDG_CONFIG_HOME/lookinfo/config.toml`.
3. `$HOME/.config/lookinfo/config.toml`.
4. Built-in defaults.

Use `$XDG_CACHE_HOME/lookinfo`, falling back to `$HOME/.cache/lookinfo`.

Never assume `/home/akumaa` exists on another machine.

### 2.3 Choose and document a format

Use TOML, JSON, or another small documented format. Provide `config/lookinfo.example.toml`, for example:

```toml
latitude = 31.1044
longitude = 77.1666
wallpaper_directory = "/home/example/Wallpapers"
quotes_file = "/home/example/.local/share/lookinfo/quotes.txt"
weather_interval_seconds = 120
memory_interval_seconds = 30
battery_interval_seconds = 60
wallpaper_interval_seconds = 3
quote_interval_seconds = 4
```

### 2.4 Validate configuration

Reject invalid values during startup:

- Latitude must be between -90 and 90.
- Longitude must be between -180 and 180.
- Intervals must be greater than zero.
- Required paths must be valid or creatable.
- The quote file must exist, or produce a clear warning.

Return errors that include the setting name and invalid value.

### 2.5 Test configuration

Test:

- Built-in defaults
- Valid configuration files
- Missing optional files
- Invalid coordinates
- Invalid intervals
- Unknown settings
- Malformed files

### Acceptance criteria

- The application starts with defaults on a clean Linux account.
- Every user-specific path is configurable.
- Changing an interval does not require recompilation.
- Invalid configuration stops startup with a useful message.

## Phase 3: Make filesystem output safe

The current writer opens the final output path directly. A reader can observe an empty or partially written cache file.

### 3.1 Create parent directories

Before writing a cache file, create its parent directories and report failures.

### 3.2 Implement atomic writes

Write to a temporary file in the same directory, flush and close it, then rename it over the destination. Use a unique temporary name and clean it up on failure.

The temporary file must be on the same filesystem so the rename is atomic.

### 3.3 Handle partial writes

The current implementation assumes one `write()` call writes all bytes. Replace it with a loop or a verified stream-based implementation.

### 3.4 Define permissions

Use user-only permissions such as `0600` unless the product explicitly requires other users to read cache files.

### 3.5 Test the writer

Test:

- Missing parent directory
- New files
- Replacing existing files
- Empty content
- Unicode content
- Invalid paths
- No partial output after failure

### Acceptance criteria

- Consumers never observe partial output.
- The application starts with a missing cache directory.
- Write failures do not crash the process.

## Phase 4: Harden every service

Every service should follow one contract:

- `update()` returns true after successfully producing output.
- `update()` returns false on recoverable failure.
- Invalid external data is rejected safely.
- A service failure does not terminate the application.
- Last valid output is preserved unless clearing it is intentional.

### 4.1 WeatherService

Add and test:

- Connection timeout
- Request timeout
- HTTP status validation
- Empty response handling
- Malformed JSON handling
- Missing JSON fields
- Non-numeric values
- Bounded retry behavior
- Preservation of the last valid output after failure

Wrap JSON parsing in exception handling. Return a failure result instead of allowing an exception to escape.

### 4.2 BatteryService

Add and test:

- Missing power-supply directory
- No battery device
- Missing `type`
- Missing `uevent`
- Missing capacity or status
- Non-numeric capacity
- Capacity outside the expected range
- Multiple batteries

Keep the injected battery path used by tests, with `/sys/class/power_supply` as the production default.

### 4.3 MemoryService

Add and test:

- Missing meminfo file
- Missing `MemTotal`
- Missing `MemAvailable`
- Invalid numeric values
- Available memory greater than total memory
- Large values

Avoid uncaught `std::stoll` exceptions.

### 4.4 WallpaperService

Add and test:

- Missing directory
- Empty directory
- Nested directories
- Supported and unsupported extensions
- Permission errors
- One wallpaper
- Repeated updates
- Missing output parent directory

Fix the repeated-update loop so one wallpaper cannot cause an infinite loop. Allow the only candidate when it is the only candidate.

### 4.5 QuoteService

Add and test:

- Missing quote file
- Empty quote file
- One quote
- Multiple quotes
- Blank lines
- Unicode quotes
- Read errors

Use a configured absolute path rather than relying on the process working directory.

### Acceptance criteria

- Malformed external input cannot crash the process.
- Every service has success and failure tests.
- Service failures are logged.
- Cache behavior after failure is documented and tested.

## Phase 5: Redesign scheduler lifecycle

The current scheduler has an unconditional `while (true)` loop and cannot stop cleanly.

### 5.1 Add a stop mechanism

Add:

- `requestStop()`
- An atomic stop flag
- A condition variable to interrupt waiting
- A `run()` method that returns after stopping

The lifecycle should be:

```text
start -> wait for next task or stop signal -> update service -> reschedule
stop signal -> wake scheduler -> finish safe operation -> return
```

### 5.2 Handle signals

Install handlers for SIGINT and SIGTERM. The handler should only set a safe flag or notify a signal-safe mechanism. Do not allocate, log, or perform complex work in the signal handler.

### 5.3 Handle service exceptions

Wrap each service update so an unexpected exception is logged and does not kill the scheduler.

### 5.4 Avoid timing drift

Choose and document whether intervals are measured from scheduled deadlines or update completion. Detect missed deadlines and avoid rapid catch-up loops.

### 5.5 Test the scheduler

Test:

- Task ordering
- Stop before the first task
- Stop while waiting
- Stop after an update
- A service returning false
- A service throwing
- Invalid intervals

### Acceptance criteria

This must stop cleanly:

```bash
./build/lockinfo
kill -TERM <pid>
```

The process must exit without requiring SIGKILL.

## Phase 6: Add command-line behavior and logging

Provide:

```text
lookinfo [options]

Options:
  --config PATH       Use a specific configuration file
  --check-config      Validate configuration and exit
  --once              Run each service once and exit
  --version           Print version
  --help              Print help
  --verbose           Enable verbose logging
```

The `--once` mode is useful for diagnostics, CI, and users who do not want a daemon.

Logs should include timestamp, severity, service name, and error description. Do not log secrets or unnecessary personal data.

## Phase 7: Improve CMake and dependencies

### 7.1 Support build types

```bash
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
```

Keep debug symbols in Debug builds and optimize Release builds.

### 7.2 Add install rules

Install:

- The `lockinfo` executable
- Example configuration
- Default data files
- Documentation
- The systemd user service

Use `GNUInstallDirs`; avoid hard-coded install directories.

### 7.3 Keep testing optional

```bash
cmake -S . -B build -DBUILD_TESTING=ON
```

The application should still build if GoogleTest is unavailable. CI should fail clearly if its test dependency is missing.

### 7.4 Document dependencies

Document runtime and development packages for supported distributions. Keep GoogleTest development-only.

## Phase 8: Create a systemd user service

Add `packaging/systemd/lookinfo.service`.

It should:

- Run as the current user.
- Use the installed executable.
- Restart after unexpected failures.
- Stop cleanly with SIGTERM.
- Write logs to the journal.
- Not rely on the current working directory.

Document:

```bash
systemctl --user daemon-reload
systemctl --user enable --now lookinfo.service
systemctl --user status lookinfo.service
journalctl --user -u lookinfo.service
systemctl --user disable --now lookinfo.service
```

## Phase 9: Add CI quality gates

Run these for every pull request and release tag:

1. Debug build and tests with GCC.
2. Release build with GCC.
3. Debug build and tests with Clang.
4. AddressSanitizer and UndefinedBehaviorSanitizer.
5. Formatting check.
6. Static analysis.
7. Installation test.
8. Package build.

Core CI command:

```bash
cmake -S . -B build -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Phase 10: Security and reliability review

Review:

- Configuration-derived filesystem paths
- Cache and configuration permissions
- Symlink behavior during writes
- TLS verification
- URL construction and input validation
- Log privacy
- Cleanup during shutdown
- Missing or invalid HOME
- Missing /proc and /sys entries
- Disk-full and read-only filesystems
- Very large or malformed external input

Use tools such as:

```bash
clang-tidy ...
cppcheck ...
valgrind ./build/lockinfo --once
```

Run practical checks in CI.

## Phase 11: Documentation

Update `README.md` with:

- Purpose and supported platforms
- Dependencies
- Build and installation
- Configuration locations and examples
- Cache file locations and formats
- One-shot usage
- systemd setup
- Troubleshooting
- Uninstallation
- Security and privacy notes

Add `CHANGELOG.md` and document breaking changes. Verify every README command from a clean checkout.

## Phase 12: Packaging and release process

### 12.1 Add version metadata

Keep the version in one place and expose:

```bash
lookinfo --version
```

Use semantic versioning:

- Major: incompatible changes
- Minor: compatible features
- Patch: bug fixes

### 12.2 Build a release artifact

```bash
cmake -S . -B build-release \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=ON

cmake --build build-release --parallel
ctest --test-dir build-release --output-on-failure
cmake --install build-release --prefix staging
```

Test the staged installation in a clean machine or container.

### 12.3 Package in this order

1. CMake install tree.
2. Debian package for Debian/Ubuntu.
3. RPM package if needed.
4. AppImage if a more self-contained distribution is required.

Do not call a dynamically linked package fully standalone. Document runtime dependencies accurately.

### 12.4 Release checklist

- Update version.
- Update changelog.
- Run all tests.
- Run sanitizers.
- Build Release artifacts.
- Test install and uninstall.
- Test systemd operation.
- Test configuration from a clean user account.
- Create a trusted release tag.
- Publish checksums.
- Publish installation and rollback instructions.

## Recommended implementation order

Follow this order to avoid rework:

1. Configuration model and path resolution.
2. Atomic file writes and directory creation.
3. Parsing, network, and error handling.
4. Scheduler shutdown and signal handling.
5. Command-line options and logging.
6. Complete service and scheduler tests.
7. CMake install rules.
8. systemd user service.
9. CI quality gates.
10. Debian/RPM packaging.
11. Documentation.
12. Release candidate.

## Working rule for each change

For every production change:

1. Add or update tests when practical.
2. Implement the smallest focused change.
3. Build with warnings enabled.
4. Run the complete suite.
5. Run the relevant sanitizer or static analysis tool.
6. Update documentation and configuration examples.
7. Review the diff for hard-coded paths and generated files.
8. Commit with a focused message.

Do not move to packaging until configuration, shutdown, filesystem safety, and error handling are complete.

