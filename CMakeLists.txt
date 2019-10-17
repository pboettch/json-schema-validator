project(json-schema-validator
    LANGUAGES CXX)

set(PROJECT_VERSION 2.0.0)

cmake_minimum_required(VERSION 3.2)

option(BUILD_TESTS      "Build tests"    ON)
option(BUILD_EXAMPLES   "Build examples" ON)

# if used as a subdirectory just define a json-hpp-target as add_library(json-hpp INTERFACE)
# and associate the path to json.hpp via target_include_directories()
if(NOT TARGET json-hpp)
    set(NLOHMANN_JSON_DIR "" CACHE STRING "path to json.hpp")

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
    add_library(json-hpp INTERFACE)
    target_include_directories(json-hpp
        INTERFACE
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${NLOHMANN_JSON_REALPATH}>)

    install(TARGETS json-hpp EXPORT ${PROJECT_NAME}Targets)
    install(FILES ${NLOHMANN_JSON_REALPATH}/nlohmann/json.hpp DESTINATION include/nlohmann)

endif()

# and one for the validator
add_library(json-schema-validator
    src/json-schema-draft7.json.cpp
    src/json-uri.cpp
    src/json-validator.cpp)

set_target_properties(json-schema-validator
                      PROPERTIES
                          VERSION ${PROJECT_VERSION}
                          SOVERSION 1)

install(TARGETS json-schema-validator
        EXPORT ${PROJECT_NAME}Targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

install(DIRECTORY src/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h*")

target_include_directories(json-schema-validator
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

target_compile_features(json-schema-validator
    PUBLIC
        cxx_range_for) # for C++11 - flags

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(json-schema-validator
        PRIVATE
            -Wall -Wextra)
endif()

target_link_libraries(json-schema-validator
    PUBLIC
        json-hpp)
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

if(NOT TARGET json-hpp) # if used as a subdirectory do not install json-schema.hpp
    install(
        FILES
           ${CMAKE_CURRENT_SOURCE_DIR}/src/json-schema.hpp
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}/include
    )
endif()

if (BUILD_EXAMPLES)
    # simple json-schema-validator-executable
    add_executable(json-schema-validate app/json-schema-validate.cpp)
    target_link_libraries(json-schema-validate json-schema-validator)

    add_executable(readme-json-schema app/readme.cpp)
    target_link_libraries(readme-json-schema json-schema-validator)
endif()

#add_subdirectory(ng)

if (BUILD_TESTS)
    # test-zone
    enable_testing()
    add_subdirectory(test)
endif()

##---------------------------------------------------------------------------##
## Set Up the Project Targets and Config Files for CMake
##---------------------------------------------------------------------------##

# Set the install path to the cmake config files
set(INSTALL_CMAKE_DIR ${CMAKE_INSTALL_PREFIX}/lib/cmake/${PROJECT_NAME})

# Create the ConfigVersion file
include(CMakePackageConfigHelpers) # write_basic_package_version_file
write_basic_package_version_file( ${PROJECT_NAME}ConfigVersion.cmake
                                  VERSION ${PACKAGE_VERSION}
                                  COMPATIBILITY SameMajorVersion)

# Get the relative path from the INSTALL_CMAKE_DIR to the include directory
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${CMAKE_INSTALL_PREFIX}/include")


# Configure the Config.cmake file with the proper include directory
set(CONF_INCLUDE_DIRS "\${JSON_SCHEMA_VALIDATOR_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(${PROJECT_NAME}Config.cmake.in
               "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" @ONLY)

# Install the Config.cmake and ConfigVersion.cmake files
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION "${INSTALL_CMAKE_DIR}")

# Install Targets
install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        DESTINATION "${INSTALL_CMAKE_DIR}")