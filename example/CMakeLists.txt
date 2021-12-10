cmake_minimum_required(VERSION 3.16)
include(FetchContent)

project(stand-alone VERSION 1.0.0 LANGUAGES CXX)

set(JSON_VALIDATOR_BUILD_EXAMPLES OFF)
set(JSON_VALIDATOR_BUILD_TESTS OFF)

FetchContent_Declare(json_schema_validator 
GIT_REPOSITORY https://github.com/vrince/json-schema-validator
GIT_TAG 379cf8c2b4ad49fe3a21e926ffdf543b9e9da8df)
FetchContent_MakeAvailable(json_schema_validator)

add_executable(stand-alone stand-alone.cpp)
target_link_libraries(stand-alone nlohmann_json_schema_validator)