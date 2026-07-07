# LookInfo

> A modular Linux information daemon written in modern C++20.

LookInfo is a lightweight background daemon that periodically collects system and external information and publishes it through a local cache directory. Desktop applications such as Hyprlock, Waybar, Eww, or custom scripts can consume this information without implementing their own polling logic.

The goal of the project is to explore Linux systems programming, scheduling algorithms, daemon architecture, and modern C++ design while building software that is genuinely useful in a daily Linux desktop environment.

---

## Features

* Modular service-based architecture
* Priority queue scheduler for efficient task execution
* Independent update intervals for every service
* Modern C++20 implementation
* Linux filesystem integration
* Lightweight cache-based communication
* Easy to extend with new services

Current modules:

* 🌦 Weather (Open-Meteo API)
* 🔋 Battery Information
* 🧠 Memory Usage
* 💬 Random Quotes
* 🖼 Random Wallpaper Selection

---

# Why LookInfo?

Most desktop widgets repeatedly perform the same work:

* Every widget polls the weather API.
* Every widget reads `/proc` or `/sys`.
* Every widget performs its own timers.
* Every widget duplicates parsing logic.

LookInfo centralizes all of that into a single daemon.

Instead of every desktop component collecting information independently, LookInfo gathers the data once, caches the latest state, and allows any application to consume it.

This separates **data collection** from **data presentation**, making desktop integrations cleaner, faster, and easier to maintain.

---

# Architecture

```
                  Scheduler
                       │
        ┌──────────────┼──────────────┐
        │              │              │
        ▼              ▼              ▼
 WeatherService  BatteryService  MemoryService
        │              │              │
        ▼              ▼              ▼
   QuoteService   WallpaperService   ...
                │
                ▼
        ~/.cache/lookinfo/
                │
 ┌──────────────┼────────────────────┐
 ▼              ▼                    ▼
weather.txt  battery.txt      memory.txt
quotes.txt   wallpaper.txt
```

Every module implements a common `IService` interface.

The scheduler has no knowledge of weather, battery, wallpaper, or memory. Its only responsibility is deciding **when** each service should execute.

This separation keeps the scheduling logic completely independent from the services themselves.

---

# Scheduler Design

Each service has its own execution interval.

Instead of polling every service continuously, LookInfo stores scheduled tasks inside a priority queue ordered by their next execution time.

```
Pop next task
        │
        ▼
Sleep until execution time
        │
        ▼
Execute service->update()
        │
        ▼
Compute next execution time
        │
        ▼
Push task back into heap
```

This allows every service to update independently while the scheduler remains idle most of the time.

Example scheduling intervals:

| Service   |     Interval |
| --------- | -----------: |
| Memory    |   30 seconds |
| Battery   |     1 minute |
| Weather   |   30 minutes |
| Quote     |     12 hours |
| Wallpaper | Configurable |

The scheduler uses `std::chrono::steady_clock` to measure elapsed time so scheduling remains correct even if the system clock changes.

---

# Project Structure

```
include/
│
├── IService.hpp
├── scheduler.hpp
├── weather.hpp
├── batteryService.hpp
├── memoryService.hpp
├── quoteService.hpp
└── wallpaper.hpp

src/
│
├── scheduler.cpp
├── weather.cpp
├── batteryService.cpp
├── memoryService.cpp
├── quoteService.cpp
├── wallpaper.cpp
├── fileHandler.cpp
├── pathFinder.cpp
└── main.cpp
```

---

# Cache Output

LookInfo publishes the latest state into:

```
~/.cache/lookinfo/
```

Example:

```
weather.txt
battery.txt
memory.txt
quotes.txt
wallpaper.txt
```

These files can be consumed by:

* Hyprlock
* Waybar
* Eww
* Shell scripts
* Custom desktop widgets

The daemon itself is intentionally independent of any specific desktop environment.

---

# Technologies Used

* C++20
* CMake
* libcurl
* nlohmann/json
* std::filesystem
* std::chrono
* std::priority_queue
* Linux `/proc`
* Linux `/sys`

---

# Design Decisions

### Why a priority queue?

Each service has a different update interval. A priority queue efficiently keeps track of the next service that should execute, allowing the scheduler to sleep until the next task instead of continuously scanning every service.

---

### Why `std::chrono::steady_clock`?

The scheduler measures elapsed time rather than wall-clock time.

`steady_clock` is monotonic and is not affected by manual clock changes, NTP synchronization, daylight saving time, or timezone changes, making it ideal for scheduling periodic tasks.

---

### Why cache files?

Instead of tightly coupling the daemon to Hyprland, Waybar, or Hyprlock, LookInfo publishes its state into cache files under:

```
~/.cache/lookinfo
```

This makes the daemon reusable by any application capable of reading text files.

---

### Why separate services?

Every module has a single responsibility.

For example:

* Weather handles HTTP requests and JSON parsing.
* Battery reads Linux power information.
* Memory parses `/proc/meminfo`.
* Wallpaper scans local directories and selects random images.

Adding a new module only requires implementing the `IService` interface and registering it with the scheduler.

---

# Future Roadmap

## Core

* [x] Modular service architecture
* [x] Priority queue scheduler
* [x] Weather module
* [x] Battery module
* [x] Memory module
* [x] Quote module
* [x] Wallpaper module

## Planned

* [ ] Configuration file
* [ ] Logging system
* [ ] Graceful shutdown (SIGINT/SIGTERM)
* [ ] Proper daemon mode
* [ ] Unit tests
* [ ] IPC (Unix Domain Socket)
* [ ] Plugin support

## Companion Project

A separate desktop companion application is planned.

Unlike LookInfo, which only **collects and publishes information**, the companion project will **consume** that information and perform desktop actions such as:

* Applying wallpapers with `swww`
* Updating desktop widgets
* Displaying notifications
* Integrating with Hyprland components

Keeping these responsibilities separate makes the daemon reusable across different desktop environments and allows presentation logic to evolve independently.

---

# Building

```bash
git clone https://github.com/<your-username>/lookinfo.git

cd lookinfo

cmake -B build

cmake --build build

./build/lookinfo
```

---

# Motivation

This project was built to gain a deeper understanding of:

* Modern C++ design
* Linux systems programming
* Filesystem APIs
* Scheduling algorithms
* HTTP networking
* Daemon architecture
* Separation of concerns

Rather than building a collection of unrelated utilities, the goal was to design a modular system where each component has a well-defined responsibility and can evolve independently.
