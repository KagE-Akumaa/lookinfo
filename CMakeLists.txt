cmake_minimum_required(VERSION 3.20)

project(LockInfo VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)

add_executable(lockinfo
    src/lookinfo.cpp
    src/weather.cpp
)

target_include_directories(lockinfo PRIVATE include)

target_link_libraries(lockinfo PRIVATE CURL::libcurl)

target_compile_options(lockinfo PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
