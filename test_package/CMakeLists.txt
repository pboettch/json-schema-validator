cmake_minimum_required(VERSION 3.14)
project(test_package LANGUAGES CXX)

find_package(nlohmann_json_schema_validator REQUIRED)

add_executable(test_package test_package.cpp)
target_link_libraries(test_package PRIVATE nlohmann_json_schema_validator::validator)
