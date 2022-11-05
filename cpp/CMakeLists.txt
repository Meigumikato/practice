cmake_minimum_required(VERSION 3.20)

include(FetchContent)

project(cpp)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SANITIZER "-fsanitize=address")

set(COROUTINE_FLAG "-fcoroutines")

if (APPLE) 
  set(COROUTINE_FLAG "-fcoroutnes-ts")
endif()


FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.11.0
)
FetchContent_MakeAvailable(spdlog)


FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/boostorg/asio.git
  GIT_TAG boost-1.80.0
)

FetchContent_MakeAvailable(asio)
# find_package(Boost REQUIRED)


add_executable(generator)
target_compile_features(generator PRIVATE cxx_std_20)
target_link_options(generator PRIVATE ${SANITIZER})
target_sources(generator PRIVATE generator.cpp)
target_link_libraries(generator PRIVATE spdlog::spdlog)

add_executable(task)
target_compile_features(task PRIVATE cxx_std_20)
target_link_options(task PRIVATE ${SANITIZER})
target_sources(task PRIVATE task.cpp sync_wait.hpp)
target_link_libraries(task PRIVATE spdlog::spdlog)

add_executable(echo)
target_compile_features(echo PRIVATE cxx_std_20)
target_sources(echo PRIVATE echo_server.cpp)
target_link_options(echo PRIVATE ${SANITIZER})

add_executable(io_context)
target_compile_features(io_context PRIVATE cxx_std_20)
target_sources(io_context PRIVATE io_context.cpp)
target_link_options(io_context PRIVATE ${SANITIZER})
target_link_libraries(io_context PRIVATE spdlog::spdlog)
