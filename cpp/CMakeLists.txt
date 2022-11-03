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
    libcoro
    GIT_REPOSITORY https://github.com/jbaldwin/libcoro.git
    GIT_TAG        v0.6
)
FetchContent_MakeAvailable(libcoro)




add_executable(generator)
target_compile_features(generator PRIVATE cxx_std_20)
target_compile_options(generator PRIVATE 
  ${COROUTINE_FLAG}
  )
target_link_options(generator PRIVATE ${SANITIZER})
target_sources(generator PRIVATE generator.cpp)
target_link_libraries(generator PRIVATE spdlog::spdlog libcoro)

add_executable(generator)
target_compile_features(generator PRIVATE cxx_std_20)
target_compile_options(generator PRIVATE 
  ${COROUTINE_FLAG}
  )
target_link_options(generator PRIVATE ${SANITIZER})
target_sources(generator PRIVATE generator.cpp)
target_link_libraries(generator PRIVATE spdlog::spdlog libcoro)
