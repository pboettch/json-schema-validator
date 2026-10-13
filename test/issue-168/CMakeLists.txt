# recursive $ref to a schema below an unknown keyword ("myDefs" instead of "definitions")

# the unknown keyword is found after the first reference to it
add_test_simple_schema(Issue::168
                       ${CMAKE_CURRENT_SOURCE_DIR}/schema.json
                       ${CMAKE_CURRENT_SOURCE_DIR}/instance.json)
add_test_simple_schema(Issue::168-invalid
                       ${CMAKE_CURRENT_SOURCE_DIR}/schema.json
                       ${CMAKE_CURRENT_SOURCE_DIR}/instance-invalid.json)

# the unknown keyword is found before the first reference to it
add_test_simple_schema(Issue::168-stored-first
                       ${CMAKE_CURRENT_SOURCE_DIR}/schema-stored-first.json
                       ${CMAKE_CURRENT_SOURCE_DIR}/instance-stored-first.json)
add_test_simple_schema(Issue::168-stored-first-invalid
                       ${CMAKE_CURRENT_SOURCE_DIR}/schema-stored-first.json
                       ${CMAKE_CURRENT_SOURCE_DIR}/instance-stored-first-invalid.json)

set_tests_properties(Issue::168-invalid Issue::168-stored-first-invalid
                     PROPERTIES
                     WILL_FAIL 1)
