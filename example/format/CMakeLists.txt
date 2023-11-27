cmake_minimum_required(VERSION 3.24)

project(example_format LANGUAGES CXX)

include(FetchContent)

# The target check is not generally needed. It is used here because these projects are reused by the top-level project
if (NOT TARGET nlohmann_json_schema_validator::validator)
    # Note: The 3.24 cmake requirement only appears due to `FetchContent_Declare(FIND_PACKAGE_ARGS)`
    # To support earlier versions, you can replace this with `find_package` or remove `FIND_PACKAGE_ARGS`
    FetchContent_Declare(nlohmann_json_schema_validator
            GIT_REPOSITORY https://github.com/pboettch/json-schema-validator
            GIT_TAG main
            FIND_PACKAGE_ARGS CONFIG
    )
    FetchContent_MakeAvailable(nlohmann_json_schema_validator)
endif ()

add_executable(format-json-schema format.cpp)
target_link_libraries(format-json-schema PRIVATE nlohmann_json_schema_validator::validator)

# Reusing the top-level install in order to bundle these executables
if (JSON_VALIDATOR_INSTALL)
    install(TARGETS format-json-schema
            DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
