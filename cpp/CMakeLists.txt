cmake_minimum_required(VERSION 3.20)
project(cpp)

include(CTest)
include(FetchContent)
include(GoogleTest)
include(GNUInstallDirs)

enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SANITIZER "")
# set(SANITIZER "-fsanitize=address")

find_package(spdlog CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(folly REQUIRED)
find_package(gflags REQUIRED)
find_package(range-v3 CONFIG REQUIRED)


set(TEST_BASE spdlog::spdlog GTest::gtest_main)

add_subdirectory(coroutine)
add_subdirectory(range)
