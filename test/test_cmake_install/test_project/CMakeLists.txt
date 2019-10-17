# This is a simple project that tests using cmake to load the installed libraries
cmake_minimum_required(VERSION 3.2)

project(cmake_install_test LANGUAGES CXX)

set(PROJECT_VERSION 1.0.0)
enable_testing()

# Set the nlohmann json include directory used by json-schema-validator
set(NLOHMANN_JSON_DIR "" CACHE STRING "path to json.hpp")

# Set the json-schema-validator install directory
set(VALIDATOR_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../install-dir"
    CACHE STRING "The json-schema-validator install directory")

# Find the json-schema-validator package
set(CMAKE_FIND_DEBUG_MODE ON)
find_package(json-schema-validator REQUIRED HINTS ${VALIDATOR_INSTALL_DIR})

# Add simple json-schema-validator-executable
add_executable(json-schema-validate ${CMAKE_CURRENT_SOURCE_DIR}/../../../app/json-schema-validate.cpp)
target_link_libraries(json-schema-validate json-schema-validator)

# Add built-in tests function needed for issues
set(PIPE_IN_TEST_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/../../test-pipe-in.sh)
function(add_test_simple_schema name schema instance)
    add_test(
        NAME ${name}
        COMMAND ${PIPE_IN_TEST_SCRIPT}
            $<TARGET_FILE:json-schema-validate>
            ${schema}
            ${instance}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

# Run tests for issues 9, 12, 27, 48, 54
foreach(NUMBER "9" "12" "27" "48" "54")
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../issue-${NUMBER}" "${CMAKE_CURRENT_BINARY_DIR}/issue-${NUMBER}" EXCLUDE_FROM_ALL)
endforeach()