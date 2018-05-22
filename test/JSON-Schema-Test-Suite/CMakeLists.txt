set(JSON_SCHEMA_TEST_PREFIX "JSON-Suite" CACHE STRING "prefix for JSON-tests added to ctest")

# find schema-test-suite
find_path(JSON_SCHEMA_TEST_SUITE_PATH
    NAMES
        tests/draft4)

if (NOT JSON_SCHEMA_TEST_SUITE_PATH)
    message(STATUS "Set JSON_SCHEMA_TEST_SUITE_PATH to a path in which JSON-Schema-Test-Suite is located (github.com/json-schema-org/JSON-Schema-Test-Suite). Using internal test-suite which might be out of date.")
    set(JSON_SCHEMA_TEST_SUITE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(JSON_SCHEMA_TEST_SUITE_PATH)
    # json-schema-validator-tester
    add_executable(json-schema-test json-schema-test.cpp)
    target_link_libraries(json-schema-test json-schema-validator)
    target_compile_definitions(json-schema-test
        PRIVATE
            JSON_SCHEMA_TEST_SUITE_PATH="${JSON_SCHEMA_TEST_SUITE_PATH}")

    option(JSON_SCHEMA_ENABLE_OPTIONAL_TESTS "Enable optional tests of the JSONSchema Test Suite" ON)

    # create tests foreach test-file
    file(GLOB TEST_FILES ${JSON_SCHEMA_TEST_SUITE_PATH}/tests/draft4/*.json)

    foreach(TEST_FILE ${TEST_FILES})
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
        add_test(NAME "${JSON_SCHEMA_TEST_PREFIX}::${TEST_NAME}"
                 COMMAND ${PIPE_IN_TEST_SCRIPT} $<TARGET_FILE:json-schema-test> ${TEST_FILE})
    endforeach()

    if (JSON_SCHEMA_ENABLE_OPTIONAL_TESTS)
        file(GLOB OPT_TEST_FILES ${JSON_SCHEMA_TEST_SUITE_PATH}/tests/draft4/optional/*.json)

        foreach(TEST_FILE ${OPT_TEST_FILES})
            get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
            add_test(NAME "${JSON_SCHEMA_TEST_PREFIX}::Optional::${TEST_NAME}"
                     COMMAND ${PIPE_IN_TEST_SCRIPT} $<TARGET_FILE:json-schema-test> ${TEST_FILE})
        endforeach()

        # some optional tests will fail as well.
        set_tests_properties(JSON-Suite::Optional::bignum
                             JSON-Suite::Optional::ecmascript-regex
                             JSON-Suite::Optional::format
                             PROPERTIES
                                WILL_FAIL ON)
    endif()
else()
endif()
