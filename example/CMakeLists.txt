cmake_minimum_required(VERSION 3.16)
include(FetchContent)

project(stand-alone VERSION 1.0.0 LANGUAGES CXX)

FetchContent_Declare(json_schema_validator 
GIT_REPOSITORY https://github.com/pboettch/json-schema-validator.git 
GIT_TAG v2.1.0)
FetchContent_MakeAvailable(json_schema_validator)

add_executable(stand-alone stand-alone.cpp)
target_link_libraries(stand-alone nlohmann_json_schema_validator)