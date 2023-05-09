# This is a simple project that tests using cmake to load the installed libraries
cmake_minimum_required(VERSION 3.14)

project(test_find_package LANGUAGES CXX)

set(CMAKE_FIND_DEBUG_MODE ON)
find_package(nlohmann_json_schema_validator REQUIRED)

if (NOT TARGET nlohmann_json_schema_validator::validator)
	message(FATAL_ERROR "Missing target nlohmann_json_schema_validator::validator")
endif ()
