cmake_minimum_required(VERSION 3.20)

project(LookInfo VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
include(FetchContent)

# -------------------------------------------------------------
# Dependencies
# -------------------------------------------------------------

find_package(CURL REQUIRED)

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.4.0
)

FetchContent_MakeAvailable(tomlplusplus)

# -------------------------------------------------------------
# Core library
# -------------------------------------------------------------

set(LOOKINFO_SOURCES
    src/configLoader.cpp
    src/weather.cpp
    src/quoteService.cpp
    src/fileHandler.cpp
    src/pathFinder.cpp
    src/batteryService.cpp
    src/memoryService.cpp
    src/scheduler.cpp
    src/wallpaper.cpp
    src/configLoader.cpp
)

add_library(lookinfo_core STATIC
    ${LOOKINFO_SOURCES}
)

target_include_directories(lookinfo_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(lookinfo_core
    PUBLIC
        CURL::libcurl
        tomlplusplus::tomlplusplus
)

target_compile_options(lookinfo_core
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

# -------------------------------------------------------------
# Application executable
# -------------------------------------------------------------

add_executable(lookinfo
    src/main.cpp
)

target_link_libraries(lookinfo
    PRIVATE
        lookinfo_core
)

target_compile_options(lookinfo
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

# -------------------------------------------------------------
# Tests
# -------------------------------------------------------------

if(BUILD_TESTING)
    find_package(GTest QUIET)

    if(GTest_FOUND)
        file(GLOB TEST_SOURCES CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
        )

        if(TEST_SOURCES)
            include(GoogleTest)

            add_executable(lookinfo_tests
                ${TEST_SOURCES}
            )

            target_link_libraries(lookinfo_tests
                PRIVATE
                    lookinfo_core
                    GTest::gtest_main
            )

            target_compile_options(lookinfo_tests
                PRIVATE
                    -Wall
                    -Wextra
                    -Wpedantic
            )

            gtest_discover_tests(lookinfo_tests)
        else()
            message(STATUS
                "GoogleTest found, but no test sources were found in tests/"
            )
        endif()
    else()
        message(STATUS
            "GoogleTest not found; building without tests"
        )
    endif()
endif()
