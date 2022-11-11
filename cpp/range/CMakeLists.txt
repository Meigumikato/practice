cmake_minimum_required(VERSION 3.20)

project(range)

enable_testing()

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE rapidjson range-v3::range-v3)
gtest_discover_tests(${PROJECT_NAME})
