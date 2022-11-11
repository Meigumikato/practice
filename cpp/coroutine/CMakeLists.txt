cmake_minimum_required(VERSION 3.20)

project(coroutine)

enable_testing()

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
