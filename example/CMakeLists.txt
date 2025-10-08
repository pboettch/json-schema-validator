add_executable(readme-json-schema readme.cpp)
target_link_libraries(readme-json-schema nlohmann_json_schema_validator)

add_executable(format-json-schema format.cpp)
target_link_libraries(format-json-schema nlohmann_json_schema_validator)

if (JSON_VALIDATOR_INSTALL)
    install(TARGETS readme-json-schema format-json-schema
            DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
