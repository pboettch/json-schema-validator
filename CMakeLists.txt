# Prefers json submodule
# Add Hunter support (Disabled by default)
option(HUNTER_ENABLED "Enable Hunter package manager support" OFF)
if(HUNTER_ENABLED)
    include("cmake/HunterGate.cmake")
    HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.262.tar.gz"
        SHA1 "eb51e633e08cdbe2153caf255e9c23968fecb29d"
    )
endif()

project(nlohmann_json_schema_validator
        LANGUAGES CXX)

set(PROJECT_VERSION 2.1.1)

cmake_minimum_required(VERSION 3.2)

option(BUILD_TESTS    "Build tests"    ON)
option(BUILD_EXAMPLES "Build examples" ON)

# Add nlohmann_json::nlohmann_json using Hunter package manager
if(HUNTER_ENABLED)
    hunter_add_package(nlohmann_json)
endif()

include_directories(../json/single_include)

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

# if used as a sub-directory, do not create install-rules -
# because of the dependency to nlohmann_json.
set(JSON_VALIDATOR_INSTALL ON)

# here we decice how nlohmann::json is found and used to build this project

# first, check whether a nlohmann_json::nlohmann_json target exists already
# -> we are used as a sub-directory from within another project
if(TARGET nlohmann_json::nlohmann_json)
    message(STATUS "Found nlohmann_json::nlohmann_json-target - linking with it")
    target_link_libraries(
        nlohmann_json_schema_validator
        PUBLIC nlohmann_json::nlohmann_json)

    set(JSON_VALIDATOR_INSTALL OFF)
    set(BUILD_TESTS OFF)
    set(BUILD_EXAMPLES OFF)

elseif(TARGET nlohmann_json) # or nlohmann_json, we are used a sub-project next to nlohmann-json's git repo
    message(STATUS "Found nlohmann_json-target - linking with it")
    target_link_libraries(
        nlohmann_json_schema_validator
        PUBLIC nlohmann_json)
    set(JSON_VALIDATOR_INSTALL OFF)
    set(BUILD_TESTS OFF)
    set(BUILD_EXAMPLES OFF)

else()
    if (NOT IS_ABSOLUTE ${nlohmann_json_DIR}) # make nlohmann_json_DIR absolute
        get_filename_component(nlohmann_json_DIR
                               "${CMAKE_CURRENT_BINARY_DIR}/${nlohmann_json_DIR}"
                               REALPATH)
    endif()

    set(nlohmann_json_DIR ../../${CMAKE_CURRENT_SOURCE_DIR}/external/json/)
    set(nlohmann_json_orignal_DIR ${nlohmann_json_DIR}) # save path for later use

    # find nlohmann_json-cmake-package
    find_package(nlohmann_json QUIET)

    if(TARGET nlohmann_json::nlohmann_json)
        message(STATUS "Found nlohmann_json-cmake-package - linking with it")
        target_link_libraries(
            nlohmann_json_schema_validator
            PUBLIC
            nlohmann_json::nlohmann_json)
    else()

        # find nlohmann/json.hpp

        message(STATUS ${nlohmann_json_orignal_DIR})

        find_path(JSON_HPP nlohmann/json.hpp
                  PATHS ${nlohmann_json_orignal_DIR})

        if(EXISTS ${JSON_HPP}/nlohmann/json.hpp)
            message(STATUS "Found nlohmann/json.hpp in given path: ${JSON_HPP}")
            target_include_directories(
                nlohmann_json_schema_validator
                PUBLIC $<BUILD_INTERFACE:${JSON_HPP}>)
        # else()
        #     message(FATAL_ERROR "could not find nlohmann/json.hpp or any related cmake-target. Please set nlohmann_json_DIR.")
        endif()

        # nlohmann_json_DIR has to be reset (for later use in tests)
        set(nlohmann_json_DIR ${JSON_HPP})
    endif()
endif()

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

if(JSON_VALIDATOR_INSTALL)
    install(TARGETS nlohmann_json_schema_validator
            EXPORT ${PROJECT_NAME}Targets
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin)

    install(FILES src/nlohmann/json-schema.hpp
            DESTINATION include/nlohmann)
endif()

if (BUILD_EXAMPLES)
    # simple nlohmann_json_schema_validator-executable
    add_executable(json-schema-validate app/json-schema-validate.cpp)
    target_link_libraries(json-schema-validate nlohmann_json_schema_validator)

    add_executable(readme-json-schema app/readme.cpp)
    target_link_libraries(readme-json-schema nlohmann_json_schema_validator)

    add_executable(format-json-schema app/format.cpp)
    target_link_libraries(format-json-schema nlohmann_json_schema_validator)

    install(TARGETS json-schema-validate readme-json-schema
            DESTINATION bin)
endif()

if (BUILD_TESTS)
    # test-zone
    enable_testing()
    add_subdirectory(test)
endif()

if(JSON_VALIDATOR_INSTALL)
    # Set Up the Project Targets and Config Files for CMake

    # Set the install path to the cmake config files (Relative, so install works correctly under Hunter as well)
    set(INSTALL_CMAKE_DIR "lib/cmake/${PROJECT_NAME}")
    set(INSTALL_CMAKEDIR_ROOT share/cmake)

    # Install Targets
    install(EXPORT ${PROJECT_NAME}Targets
            FILE ${PROJECT_NAME}Targets.cmake
            DESTINATION "${INSTALL_CMAKE_DIR}")


    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
        )


    configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        INSTALL_DESTINATION ${INSTALL_CMAKEDIR_ROOT}/${PROJECT_NAME}
        )

    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION
            ${INSTALL_CMAKE_DIR}
        )


endif()
