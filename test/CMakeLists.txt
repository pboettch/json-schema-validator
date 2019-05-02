set(PIPE_IN_TEST_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test-pipe-in.sh)

# built-in tests
function(add_test_simple_schema name schema instance)
    add_test(
        NAME ${name}
        COMMAND ${PIPE_IN_TEST_SCRIPT}
            $<TARGET_FILE:json-schema-validate>
            ${schema}
            ${instance}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

file(GLOB TEST_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/*)

foreach(DIR ${TEST_DIRS})
    if(IS_DIRECTORY ${DIR})
        add_subdirectory(${DIR})
    endif()
endforeach()

add_executable(uri uri.cpp)
target_link_libraries(uri json-schema-validator)

add_executable(errors errors.cpp)
target_link_libraries(errors json-schema-validator)

add_test(NAME uri COMMAND uri)


# Unit test for string format checks
add_executable("string-format-check-test" "string-format-check-test.cpp")
target_include_directories("string-format-check-test" PRIVATE "${PROJECT_SOURCE_DIR}/src/")
target_link_libraries("string-format-check-test" json-schema-validator)

add_test(NAME "string-format-check-test" COMMAND "string-format-check-test")
