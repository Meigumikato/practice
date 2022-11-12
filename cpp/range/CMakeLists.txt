cmake_minimum_required(VERSION 3.20)

enable_testing()

project(range)


add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE range-v3::range-v3)
gtest_discover_tests(${PROJECT_NAME})
