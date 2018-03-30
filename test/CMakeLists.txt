set(PIPE_IN_TEST_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test-pipe-in.sh)

# built-in tests
function(add_test_simple_schema name schema instance)
    add_test(
        NAME ${name}
        COMMAND ${PIPE_IN_TEST_SCRIPT}
            $<TARGET_FILE:json-schema-validate>
            ${schema}
            ${instance})
endfunction()

file(GLOB TEST_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/*)

foreach(DIR ${TEST_DIRS})
    if(IS_DIRECTORY ${DIR})
        add_subdirectory(${DIR})
    endif()
endforeach()

# find schema-test-suite
find_path(JSON_SCHEMA_TEST_SUITE_PATH
    NAMES
        tests/draft4)

set(JSON_SCHEMA_TEST_PREFIX "JSON-Suite" CACHE STRING "prefix for JSON-tests added to ctest")

if(JSON_SCHEMA_TEST_SUITE_PATH)
    # json-schema-validator-tester
    add_executable(json-schema-test ${CMAKE_SOURCE_DIR}/app/json-schema-test.cpp)
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
            COMMAND ${PIPE_IN_TEST_SCRIPT} $<TARGET_FILE:json-schema-test> ${TEST_FILE}
        )
    endforeach()

    if (JSON_SCHEMA_ENABLE_OPTIONAL_TESTS)
        file(GLOB OPT_TEST_FILES ${JSON_SCHEMA_TEST_SUITE_PATH}/tests/draft4/optional/*.json)

        foreach(TEST_FILE ${OPT_TEST_FILES})
            get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
            add_test(
                NAME "${JSON_SCHEMA_TEST_PREFIX}::Optional::${TEST_NAME}"
                COMMAND ${PIPE_IN_TEST_SCRIPT} $<TARGET_FILE:json-schema-test> ${TEST_FILE}
            )
        endforeach()
    endif()
else()
    message(STATUS "Consider setting JSON_SCHEMA_TEST_SUITE_PATH to a path in which JSON-Schema-Test-Suite is located (github.com/json-schema-org/JSON-Schema-Test-Suite).")
endif()
