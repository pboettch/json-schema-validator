cmake_minimum_required(VERSION 3.2)

project(simple-json-validator-as-submodule-test)

# absolute-path
set(PATH_TO_NLOHMANN_JSON_REPO $ENV{HOME}/devel/upstream/json)

# build shared library
set(BUILD_SHARED_LIBS ON)

# JSON library
option(JSON_BuildTests OFF)
add_subdirectory(${PATH_TO_NLOHMANN_JSON_REPO}
                 json-binary-dir
                 EXCLUDE_FROM_ALL)

# JSON SCHEMA VALIDATOR library
option(BUILD_TESTS OFF)
add_subdirectory(../..
                 json-validator-binary-dir
                 EXCLUDE_FROM_ALL)

add_executable(validate validate.cpp)
target_link_libraries(validate nlohmann_json_schema_validator)
