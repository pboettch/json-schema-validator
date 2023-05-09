# This is a simple project that tests using cmake to load the installed libraries
cmake_minimum_required(VERSION 3.14)

project(test_fetch_content LANGUAGES CXX)


FetchContent_Declare(nlohmann_json_schema_validator
		GIT_REPOSITORY https://github.com/pboettch/json-schema-validator
		GIT_TAG main
		)
FetchContent_MakeAvailable(nlohmann_json_schema_validator)

if (NOT TARGET nlohmann_json_schema_validator::validator)
	message(FATAL_ERROR "Missing target nlohmann_json_schema_validator::validator")
endif ()
