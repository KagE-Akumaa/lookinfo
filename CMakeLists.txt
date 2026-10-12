cmake_minimum_required(VERSION 3.20)

project(LockInfo VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)

set(LOCKINFO_SOURCES
    src/weather.cpp
    src/quoteService.cpp
    src/fileHandler.cpp
    src/pathFinder.cpp
    src/batteryService.cpp
    src/memoryService.cpp
    src/scheduler.cpp
    src/wallpaper.cpp
)

add_library(lockinfo_core STATIC ${LOCKINFO_SOURCES})

target_include_directories(lockinfo_core PUBLIC include)

target_link_libraries(lockinfo_core PUBLIC CURL::libcurl)

target_compile_options(lockinfo_core PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_executable(lockinfo src/main.cpp)

target_link_libraries(lockinfo PRIVATE lockinfo_core)

include(CTest)

if(BUILD_TESTING)
    find_package(GTest QUIET)

    if(GTest_FOUND)
        file(GLOB TEST_SOURCES CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
        )

        if(TEST_SOURCES)
            include(GoogleTest)

            add_executable(lockinfo_tests ${TEST_SOURCES})
            target_link_libraries(lockinfo_tests
                PRIVATE
                    lockinfo_core
                    GTest::gtest_main
            )

            gtest_discover_tests(lockinfo_tests)
        else()
            message(STATUS "GoogleTest found, but no test sources were found in tests/")
        endif()
    else()
        message(STATUS "GoogleTest not found; building without tests")
    endif()
endif()
