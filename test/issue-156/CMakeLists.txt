# the same file referenced with and without "./" and "../" in its path is loaded once,
# with an absolute $id (root.json, v1.json) or a relative one resolving to its own URL (common.json, v2.json)
add_test_simple_schema(Issue::156
                       ${CMAKE_CURRENT_SOURCE_DIR}/schemas/root.json
                       ${CMAKE_CURRENT_SOURCE_DIR}/instance.json)
add_test_simple_schema(Issue::156-invalid
                       ${CMAKE_CURRENT_SOURCE_DIR}/schemas/root.json
                       ${CMAKE_CURRENT_SOURCE_DIR}/instance-invalid.json)
set_tests_properties(Issue::156-invalid
                     PROPERTIES
                     WILL_FAIL 1)
