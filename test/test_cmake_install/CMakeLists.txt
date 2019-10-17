set(PIPE_IN_TEST_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/../test-pipe-in.sh)

# Configure install script
configure_file(test_cmake_install.sh.in
               ${CMAKE_CURRENT_BINARY_DIR}/test_cmake_install.sh @ONLY)

# built-in tests
add_test(NAME test_cmake_install
         COMMAND ${PIPE_IN_TEST_SCRIPT}
                 bash ${CMAKE_CURRENT_BINARY_DIR}/test_cmake_install.sh
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
