project(json-schema-validator CXX)

cmake_minimum_required(VERSION 3.2)

# if used as a subdirectory just define a json-hpp-target (INTERFACE)
if(NOT TARGET json-hpp)
# find nlohmann's json.hpp
    find_path(NLOHMANN_JSON_DIR
        NAMES
            json.hpp)

    if(NOT NLOHMANN_JSON_DIR)
        message(FATAL_ERROR "please set NLOHMANN_JSON_DIR to a path in which NLohmann's json.hpp can be found.")
    endif()

# create an interface-library for simple cmake-linking
    add_library(json-hpp INTERFACE)
    target_include_directories(json-hpp
        INTERFACE
            ${NLOHMANN_JSON_DIR})
endif()

# and one for the validator
add_library(json-schema-validator
    src/json-schema-draft4.json.cpp
    src/json-uri.cpp
    src/json-validator.cpp)
target_include_directories(json-schema-validator
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_options(json-schema-validator
    PUBLIC
        -Wall -Wextra) # bad, better use something else based on compiler type
target_link_libraries(json-schema-validator
    PUBLIC
        json-hpp)

install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/json-schema-validator.hpp
    DESTINATION
        ${CMAKE_INSTALL_PREFIX}/include
)

# simple json-schema-validator-executable
add_executable(json-schema-validate app/json-schema-validate.cpp)
target_link_libraries(json-schema-validate json-schema-validator)

# test-zone
enable_testing()

# find schema-test-suite
find_path(JSON_SCHEMA_TEST_SUITE_PATH
    NAMES
        tests/draft4)

set(JSON_SCHEMA_TEST_PREFIX "JSON-Suite" CACHE STRING "prefix for JSON-tests added to ctest")

if(JSON_SCHEMA_TEST_SUITE_PATH)
    # json-schema-validator-tester
    add_executable(json-schema-test app/json-schema-test.cpp)
    target_link_libraries(json-schema-test json-schema-validator)
    target_compile_definitions(json-schema-test
        PRIVATE
            JSON_SCHEMA_TEST_SUITE_PATH="${JSON_SCHEMA_TEST_SUITE_PATH}")

    option(JSON_SCHEMA_ENABLE_OPTIONAL_TESTS "Enable optional tests of the JSONSchema Test Suite" ON)

    # create tests foreach test-file
    file(GLOB TEST_FILES ${JSON_SCHEMA_TEST_SUITE_PATH}/tests/draft4/*.json)

    foreach(TEST_FILE ${TEST_FILES})
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
        add_test(
            NAME "${JSON_SCHEMA_TEST_PREFIX}::${TEST_NAME}"
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test.sh $<TARGET_FILE:json-schema-test> ${TEST_FILE}
        )
    endforeach()

    if (JSON_SCHEMA_ENABLE_OPTIONAL_TESTS)
        file(GLOB OPT_TEST_FILES ${JSON_SCHEMA_TEST_SUITE_PATH}/tests/draft4/optional/*.json)

        foreach(TEST_FILE ${OPT_TEST_FILES})
            get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
            add_test(
                NAME "${JSON_SCHEMA_TEST_PREFIX}::Optional::${TEST_NAME}"
                COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test.sh $<TARGET_FILE:json-schema-test> ${TEST_FILE}
            )
        endforeach()
    endif()
else()
    message(STATUS "Consider setting JSON_SCHEMA_TEST_SUITE_PATH to a path in which JSON-Schema-Test-Suite is located (github.com/json-schema-org/JSON-Schema-Test-Suite).")
endif()



