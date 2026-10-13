# Runs a program with its arguments, the last argument is the file used as its standard input:
#   cmake -P test-pipe-in.cmake <program> [<arguments>...] <input-file>
# Fails if the program fails. A portable replacement for a shell's "program arguments < input-file".

# CMAKE_ARGV0 is cmake, CMAKE_ARGV1 is -P, CMAKE_ARGV2 is this script
math(EXPR last "${CMAKE_ARGC} - 1")
math(EXPR last_argument "${CMAKE_ARGC} - 2")

set(command "")
foreach (i RANGE 3 ${last_argument})
    list(APPEND command "${CMAKE_ARGV${i}}")
endforeach ()

execute_process(COMMAND ${command}
                INPUT_FILE "${CMAKE_ARGV${last}}"
                RESULT_VARIABLE result)

if (NOT result EQUAL 0)
    message(FATAL_ERROR "${command} < ${CMAKE_ARGV${last}} failed: ${result}")
endif ()
