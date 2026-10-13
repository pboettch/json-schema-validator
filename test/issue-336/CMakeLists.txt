# keys containing '/' or '~' below unknown keywords (here: siblings of $ref) must be escaped
add_test_simple_schema(Issue::336
                       ${CMAKE_CURRENT_SOURCE_DIR}/schema.json
                       ${CMAKE_CURRENT_SOURCE_DIR}/instance.json)

# a $ref into such an unknown keyword is resolved
add_test_simple_schema(Issue::336-invalid
                       ${CMAKE_CURRENT_SOURCE_DIR}/schema.json
                       ${CMAKE_CURRENT_SOURCE_DIR}/instance-invalid.json)
set_tests_properties(Issue::336-invalid
                     PROPERTIES
                     WILL_FAIL 1)
