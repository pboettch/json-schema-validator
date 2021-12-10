cmake_minimum_required(VERSION 3.16)

option(JSON_VALIDATOR_BUILD_TESTS    "Build tests"    OFF)
option(JSON_VALIDATOR_BUILD_EXAMPLES "Build examples" OFF)

# the project
project(nlohmann_json_schema_validator
        LANGUAGES CXX)

set(PROJECT_VERSION 2.2.0)
include(FetchContent)

# find nlohmann_json if not found simply fetch it
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json NOT FOUND, fetching version v3.10.4")
    FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
    GIT_TAG v3.10.4)
    FetchContent_MakeAvailable(json)
endif()

# the library
add_library(nlohmann_json_schema_validator
    src/json-schema-draft7.json.cpp
    src/json-uri.cpp
    src/json-validator.cpp
    src/json-patch.cpp
    src/string-format-check.cpp)

target_include_directories(nlohmann_json_schema_validator
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

target_compile_features(nlohmann_json_schema_validator
    PUBLIC
        cxx_range_for) # for C++11 - flags

set_target_properties(nlohmann_json_schema_validator
                      PROPERTIES
                          VERSION ${PROJECT_VERSION}
                          SOVERSION 1)

target_link_libraries(
    nlohmann_json_schema_validator
    PUBLIC nlohmann_json::nlohmann_json)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
   "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(nlohmann_json_schema_validator
        PRIVATE
            -Wall -Wextra -Wshadow)
endif()

if(BUILD_SHARED_LIBS)
    target_compile_definitions(nlohmann_json_schema_validator
        PRIVATE
            -DJSON_SCHEMA_VALIDATOR_EXPORTS)
endif()

# regex with boost if gcc < 4.9 - default is std::regex
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.9.0")
        find_package(Boost COMPONENTS regex)
        if(NOT Boost_FOUND)
            message(STATUS "GCC less then 4.9 and boost-regex NOT found - no regex used")
            target_compile_definitions(nlohmann_json_schema_validator PRIVATE -DJSON_SCHEMA_NO_REGEX)
        else()
            message(STATUS "GCC less then 4.9 and boost-regex FOUND - using boost::regex")
            target_compile_definitions(nlohmann_json_schema_validator PRIVATE -DJSON_SCHEMA_BOOST_REGEX)
            target_include_directories(nlohmann_json_schema_validator PRIVATE ${Boost_INCLUDE_DIRS})
            target_link_libraries(nlohmann_json_schema_validator PRIVATE ${Boost_LIBRARIES})
        endif()
    endif()
endif()

if (JSON_VALIDATOR_BUILD_EXAMPLES)
    # simple nlohmann_json_schema_validator-executable
    add_executable(json-schema-validate app/json-schema-validate.cpp)
    target_link_libraries(json-schema-validate nlohmann_json_schema_validator)

    add_executable(readme-json-schema app/readme.cpp)
    target_link_libraries(readme-json-schema nlohmann_json_schema_validator)

    add_executable(format-json-schema app/format.cpp)
    target_link_libraries(format-json-schema nlohmann_json_schema_validator)

    install(TARGETS json-schema-validate DESTINATION bin)
endif()

if (JSON_VALIDATOR_BUILD_TESTS)
    # test-zone
    enable_testing()
    add_subdirectory(test)
endif()

# installation
install(TARGETS nlohmann_json_schema_validator
        EXPORT ${PROJECT_NAME}Targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

install(FILES src/nlohmann/json-schema.hpp
        DESTINATION include/nlohmann)


