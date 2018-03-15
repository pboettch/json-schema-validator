project(json-schema-validator CXX)

cmake_minimum_required(VERSION 3.2)

set(PROJECT_VERSION 1.0.0)

# if used as a subdirectory just define a json-hpp-target as add_library(json-hpp INTERFACE)
# and associate the path to json.hpp via target_include_directories()
if(NOT TARGET json-hpp)
    set(NLOHMANN_JSON_DIR "" CACHE STRING "path to json.hpp")
    set(ADD_INSTALL_TARGET TRUE)

    # find nlohmann's json.hpp
    find_path(JSON_HPP nlohmann/json.hpp
        PATHS
            ${NLOHMANN_JSON_DIR}
            ${CMAKE_BINARY_DIR}/${NLOHMANN_JSON_DIR}) # in case it is a relative path

    # get the full, real path
    get_filename_component(NLOHMANN_JSON_REALPATH ${JSON_HPP} REALPATH)

    if(NOT EXISTS ${NLOHMANN_JSON_REALPATH}/nlohmann/json.hpp)
        message(FATAL_ERROR "please set NLOHMANN_JSON_DIR to a path in which NLohmann's json.hpp can be found. Looking for nlohmann/json.hpp in '${NLOHMANN_JSON_REALPATH}")
    endif()

    # create an interface-library for simple cmake-linking
    include_directories(${NLOHMANN_JSON_REALPATH})
    add_library(json-hpp INTERFACE)
endif()

# and one for the validator
include_directories(src)
add_library(${PROJECT_NAME}
    src/json-schema-draft4.json.cpp
    src/json-uri.cpp
    src/json-validator.cpp)

target_compile_features(json-schema-validator
    PUBLIC
        cxx_range_for) # for C++11 - flags
# Enable more compiler warnings, except when using Visual Studio compiler
if(NOT MSVC)
    target_compile_options(json-schema-validator
        PUBLIC
            -Wall -Wextra)
endif()
if(BUILD_SHARED_LIBS)
    target_compile_definitions(json-schema-validator
        PRIVATE
            -DJSON_SCHEMA_VALIDATOR_EXPORTS)
endif()

# regex with boost if gcc < 4.9 - default is std::regex
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.9.0")
        find_package(Boost COMPONENTS regex)
        if(NOT Boost_FOUND)
            message(STATUS "GCC less then 4.9 and boost-regex NOT found - no regex used")
            target_compile_definitions(json-schema-validator PRIVATE -DJSON_SCHEMA_NO_REGEX)
        else()
            message(STATUS "GCC less then 4.9 and boost-regex FOUND - using boost::regex")
            target_compile_definitions(json-schema-validator PRIVATE -DJSON_SCHEMA_BOOST_REGEX)
            target_include_directories(json-schema-validator PRIVATE ${Boost_INCLUDE_DIRS})
            target_link_libraries(json-schema-validator PRIVATE ${Boost_LIBRARIES})
        endif()
    endif()
endif()

if(ADD_INSTALL_TARGET) # if used as a subdirectory do not install json-schema.hpp
    include(GenerateExportHeader)
    generate_export_header(${PROJECT_NAME})
    set_property(TARGET ${PROJECT_NAME} PROPERTY VERSION ${PROJECT_VERSION})
    set_property(TARGET ${PROJECT_NAME} PROPERTY SOVERSION 1)
    set_property(TARGET ${PROJECT_NAME} PROPERTY
      INTERFACE_${PROJECT_NAME}_MAJOR_VERSION 1)
    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY
      COMPATIBLE_INTERFACE_STRING ${PROJECT_NAME}_MAJOR_VERSION)
    
    install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
      RUNTIME DESTINATION bin
      INCLUDES DESTINATION include)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/json-schema.hpp
        DESTINATION include
        COMPONENT Devel)

    include(CMakePackageConfigHelpers)
    set(ConfigPackageLocation lib/cmake/${PROJECT_NAME})
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/cmake-config/${PROJECT_NAME}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion)
    export(EXPORT "${PROJECT_NAME}Targets"
        FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake-config/${PROJECT_NAME}Targets.cmake"
        NAMESPACE ${PROJECT_NAME}::)
    configure_package_config_file(cmake/config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/cmake-config/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION ${ConfigPackageLocation})
    install(EXPORT "${PROJECT_NAME}Targets"
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${ConfigPackageLocation})
    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/cmake-config/${PROJECT_NAME}Config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/cmake-config/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${ConfigPackageLocation}
        COMPONENT Devel)
endif()

# simple json-schema-validator-executable
add_executable(json-schema-validate app/json-schema-validate.cpp)
target_link_libraries(json-schema-validate json-schema-validator)

# test-zone
enable_testing()

# find schema-test-suite
find_path(JSON_SCHEMA_TEST_SUITE_PATH
    NAMES
        tests/draft4)

set(JSON_SCHEMA_TEST_PREFIX "JSON-Suite" CACHE STRING "prefix for JSON-tests added to ctest")

if(JSON_SCHEMA_TEST_SUITE_PATH)
    # json-schema-validator-tester
    add_executable(json-schema-test app/json-schema-test.cpp)
    target_link_libraries(json-schema-test json-schema-validator)
    target_compile_definitions(json-schema-test
        PRIVATE
            JSON_SCHEMA_TEST_SUITE_PATH="${JSON_SCHEMA_TEST_SUITE_PATH}")

    option(JSON_SCHEMA_ENABLE_OPTIONAL_TESTS "Enable optional tests of the JSONSchema Test Suite" ON)

    # create tests foreach test-file
    file(GLOB TEST_FILES ${JSON_SCHEMA_TEST_SUITE_PATH}/tests/draft4/*.json)

    foreach(TEST_FILE ${TEST_FILES})
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
        add_test(
            NAME "${JSON_SCHEMA_TEST_PREFIX}::${TEST_NAME}"
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test.sh $<TARGET_FILE:json-schema-test> ${TEST_FILE}
        )
    endforeach()

    if (JSON_SCHEMA_ENABLE_OPTIONAL_TESTS)
        file(GLOB OPT_TEST_FILES ${JSON_SCHEMA_TEST_SUITE_PATH}/tests/draft4/optional/*.json)

        foreach(TEST_FILE ${OPT_TEST_FILES})
            get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
            add_test(
                NAME "${JSON_SCHEMA_TEST_PREFIX}::Optional::${TEST_NAME}"
                COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test.sh $<TARGET_FILE:json-schema-test> ${TEST_FILE}
            )
        endforeach()
    endif()
else()
    message(STATUS "Consider setting JSON_SCHEMA_TEST_SUITE_PATH to a path in which JSON-Schema-Test-Suite is located (github.com/json-schema-org/JSON-Schema-Test-Suite).")
endif()

