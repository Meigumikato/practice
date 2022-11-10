cmake_minimum_required(VERSION 3.20)

include(FetchContent)

project(cpp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SANITIZER "-fsanitize=address")

find_package(spdlog REQUIRED)
find_package(GTest REQUIRED)
find_package(folly REQUIRED)
find_package(gflags REQUIRED)

set(TEST_BASE spdlog::spdlog GTest::gtest_main)


function(test)
  add_executable(${ARGV0})
  target_compile_features(${ARGV0} PRIVATE cxx_std_20)
  target_link_options(${ARGV0} PRIVATE ${SANITIZER})
  target_sources(
    ${ARGV0}
    PRIVATE ${ARGV0}_test.cpp)
  target_link_libraries(${ARGV0} PRIVATE ${TEST_BASE} Folly::folly gflags)
  gtest_discover_tests(${ARGV0})
endfunction()


test(generator)
test(task)
test(sync_wait)
test(io_context)
test(co_spawn)

add_executable(echo)
target_compile_features(echo PRIVATE cxx_std_20)
target_sources(echo PRIVATE echo_server.cpp)
target_link_options(echo PRIVATE ${SANITIZER})
