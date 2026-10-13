set(JSON_SCHEMA_TEST_PREFIX "JSON-Suite" CACHE STRING "prefix for JSON-tests added to ctest")

set(DRAFT "draft7")

# find schema-test-suite
find_path(JSON_SCHEMA_TEST_SUITE_PATH
    NAMES
        tests/${DRAFT})

if (NOT JSON_SCHEMA_TEST_SUITE_PATH)
    message(STATUS "Set JSON_SCHEMA_TEST_SUITE_PATH to a path in which JSON-Schema-Test-Suite is located (github.com/json-schema-org/JSON-Schema-Test-Suite). Using internal test-suite which might be out of date.")
    set(JSON_SCHEMA_TEST_SUITE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(JSON_SCHEMA_TEST_SUITE_PATH)
    # json-schema-validator-tester
    add_executable(json-schema-test json-schema-test.cpp)
    target_link_libraries(json-schema-test nlohmann_json_schema_validator)
    target_compile_definitions(json-schema-test
        PRIVATE
            JSON_SCHEMA_TEST_SUITE_PATH="${JSON_SCHEMA_TEST_SUITE_PATH}")

    option(JSON_SCHEMA_ENABLE_OPTIONAL_TESTS "Enable optional tests of the JSONSchema Test Suite" ON)

    # test cases known to fail, by test file, group and test description
    set(KNOWN_FAILURES ${CMAKE_CURRENT_SOURCE_DIR}/known-failures.json)

    # one test per test file, NAME is the ctest name
    function(add_suite_test NAME TEST_FILE)
        file(RELATIVE_PATH test_file ${JSON_SCHEMA_TEST_SUITE_PATH}/tests/${DRAFT} ${TEST_FILE})
        add_test(NAME "${NAME}"
                 COMMAND ${PIPE_IN_TEST_SCRIPT} $<TARGET_FILE:json-schema-test> ${KNOWN_FAILURES} ${test_file} ${TEST_FILE})
    endfunction()

    # create tests foreach test-file
    file(GLOB TEST_FILES ${JSON_SCHEMA_TEST_SUITE_PATH}/tests/${DRAFT}/*.json)

    foreach(TEST_FILE ${TEST_FILES})
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
        add_suite_test("${JSON_SCHEMA_TEST_PREFIX}::${TEST_NAME}" ${TEST_FILE})
    endforeach()

    if (JSON_SCHEMA_ENABLE_OPTIONAL_TESTS)
        file(GLOB OPT_TEST_FILES ${JSON_SCHEMA_TEST_SUITE_PATH}/tests/${DRAFT}/optional/*.json)
        file(GLOB FORMAT_TEST_FILES ${JSON_SCHEMA_TEST_SUITE_PATH}/tests/${DRAFT}/optional/format/*.json)

        foreach(TEST_FILE ${OPT_TEST_FILES})
            get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
            add_suite_test("${JSON_SCHEMA_TEST_PREFIX}::Optional::${TEST_NAME}" ${TEST_FILE})
        endforeach()

        foreach(TEST_FILE ${FORMAT_TEST_FILES})
            get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
            add_suite_test("${JSON_SCHEMA_TEST_PREFIX}::Optional::Format::${TEST_NAME}" ${TEST_FILE})
        endforeach()

        # optional tests which fail completely, single failing test cases are in known-failures.json
        set_tests_properties(
            JSON-Suite::Optional::bignum
            JSON-Suite::Optional::non-bmp-regex
            JSON-Suite::Optional::float-overflow

            JSON-Suite::Optional::ecmascript-regex
            JSON-Suite::Optional::Format::idn-hostname
            JSON-Suite::Optional::Format::iri-reference
            JSON-Suite::Optional::Format::iri
            JSON-Suite::Optional::Format::json-pointer
            JSON-Suite::Optional::Format::relative-json-pointer
            JSON-Suite::Optional::Format::uri-reference
            JSON-Suite::Optional::Format::uri-template

            PROPERTIES
                WILL_FAIL ON)
    endif()
else()
endif()
