project(json-schema-validator CXX)

cmake_minimum_required(VERSION 3.2)

# find nlohmann's json.hpp
find_path(NLOHMANN_JSON_DIR
    NAMES
        json.hpp)

if(NOT NLOHMANN_JSON_DIR)
    message(FATAL_ERROR "please set NLOHMANN_JSON_DIR to a path in which NLohmann's json.hpp can be found.")
endif()

# create an interface-library for simple linking
add_library(json INTERFACE)
target_include_directories(json
    INTERFACE
        ${NLOHMANN_JSON_DIR})

# and one for the validator
add_library(json-schema-validator INTERFACE)
target_include_directories(json-schema-validator
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_options(json-schema-validator
    INTERFACE
        -Wall -Wextra) # bad, better use something else based on compiler type
target_link_libraries(json-schema-validator
    INTERFACE
        json)

install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/json-schema-validator.hpp
    DESTINATION
        ${CMAKE_INSTALL_PREFIX}/include
)

# simple json-schema-validator-executable
add_executable(json-schema-validate app/json-schema-validate.cpp)
target_link_libraries(json-schema-validate json-schema-validator)

# json-schema-validator-tester
add_executable(json-schema-test app/json-schema-test.cpp)
target_link_libraries(json-schema-test json-schema-validator)

enable_testing()

# find schema-test-suite
find_path(JSON_SCHEMA_TEST_SUITE_PATH
    NAMES
        tests/draft4)

if(JSON_SCHEMA_TEST_SUITE_PATH)
    # create tests foreach test-file
    file(GLOB_RECURSE TEST_FILES ${JSON_SCHEMA_TEST_SUITE_PATH}/tests/draft4/*.json)

    foreach(TEST_FILE ${TEST_FILES})
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
        add_test(
            NAME ${TEST_NAME}
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test.sh $<TARGET_FILE:json-schema-test> ${TEST_FILE}
        )
    endforeach()
else()
    message(STATUS "Please test JSON_SCHEMA_TEST_SUITE_PATH to a path in which you've cloned JSON-Schema-Test-Suite (github.com/json-schema-org/JSON-Schema-Test-Suite).")
endif()



