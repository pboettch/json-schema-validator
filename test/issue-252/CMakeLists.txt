# a schema with a file://-URI as $id, referencing another file next to it, is loaded by its absolute path
# (the tests run in the source directory)
configure_file(defs.json defs.json COPYONLY)
if (CMAKE_CURRENT_BINARY_DIR MATCHES "^/")
    set(SCHEMA_ID "file://${CMAKE_CURRENT_BINARY_DIR}/schema.json")
else () # Windows: C:/...
    set(SCHEMA_ID "file:///${CMAKE_CURRENT_BINARY_DIR}/schema.json")
endif ()
configure_file(schema.json.in schema.json @ONLY)

add_test_simple_schema(Issue::252
                       ${CMAKE_CURRENT_BINARY_DIR}/schema.json
                       ${CMAKE_CURRENT_SOURCE_DIR}/instance.json)
add_test_simple_schema(Issue::252-invalid
                       ${CMAKE_CURRENT_BINARY_DIR}/schema.json
                       ${CMAKE_CURRENT_SOURCE_DIR}/instance-invalid.json)
set_tests_properties(Issue::252-invalid
                     PROPERTIES
                     WILL_FAIL 1)
