cmake_minimum_required(VERSION 3.20)

include(FetchContent)

project(cpp)

set(SANITIZER "-fsanitize=address")

find_package(spdlog REQUIRED)
find_package(GTest REQUIRED)

add_executable(generator)
target_compile_features(generator PRIVATE cxx_std_20)
target_link_options(generator PRIVATE ${SANITIZER})
target_sources(
  generator
  INTERFACE generator.hpp
  PRIVATE generator_test.cpp)
target_link_libraries(generator PRIVATE spdlog::spdlog GTest::gtest_main)

add_executable(task)
target_compile_features(task PRIVATE cxx_std_20)
target_link_options(task PRIVATE ${SANITIZER})
target_sources(
  task
  PRIVATE task_test.cpp
  INTERFACE sync_wait.hpp task.hpp)
target_link_libraries(task PRIVATE spdlog::spdlog GTest::gtest_main)

add_executable(sync_wait)
target_compile_features(sync_wait PRIVATE cxx_std_20)
target_link_options(sync_wait PRIVATE ${SANITIZER})
target_sources(
  sync_wait
  PRIVATE sync_wait_test.cpp
  INTERFACE sync_wait.hpp)
target_link_libraries(sync_wait PRIVATE spdlog::spdlog GTest::gtest_main)

add_executable(echo)
target_compile_features(echo PRIVATE cxx_std_20)
target_sources(echo PRIVATE echo_server.cpp)
target_link_options(echo PRIVATE ${SANITIZER})

add_executable(io_context)
target_compile_features(io_context PRIVATE cxx_std_20)
target_sources(io_context PRIVATE io_context.cpp)
target_link_options(io_context PRIVATE ${SANITIZER})
target_link_libraries(io_context PRIVATE spdlog::spdlog)
