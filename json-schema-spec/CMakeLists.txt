#[==============================================================================================[
#                                       Meta-schema Generation                                  #
]==============================================================================================]

function (addSchemaFileToBuild META_SCHEMA_PATH)
    file(READ ${META_SCHEMA_PATH} FILE_CONTENT)
    string(JSON META_SCHEMA_ID GET ${FILE_CONTENT} "$id")
    string(REPLACE "(" "-^-(" FILE_CONTENT "${FILE_CONTENT}")  # work around the reality that there may be '(' within strings
    string(REPLACE ")" ")-^-" FILE_CONTENT "${FILE_CONTENT}")  # work around the reality that there may be ')' within strings
    #string(REPLACE "\n" ")\"\nR\"(" FILE_CONTENT "${FILE_CONTENT}")  # break lines into separate strings so that they are not too long for VS
    set(SOURCES ${SOURCES} PARENT_SCOPE)
    file (APPEND ${SCHEMA_CPP_FILE_NAME} "    {\"${META_SCHEMA_ID}\",\nR\"(")
    file (APPEND ${SCHEMA_CPP_FILE_NAME} ${FILE_CONTENT})
    file (APPEND ${SCHEMA_CPP_FILE_NAME} ")\"_json},\n")
endfunction()

# this logic is such that the schema header file will only be generated once; until a "clean"
set(SCHEMA_BASE_FILE_NAME builtin_schema_map)
set(SCHEMA_BASE_FILE_PATH ${CMAKE_CURRENT_BINARY_DIR}/${SCHEMA_BASE_FILE_NAME})
set(SCHEMA_HEADER_FILE_NAME ${SCHEMA_BASE_FILE_PATH}.h)
set(SCHEMA_CPP_FILE_NAME    ${SCHEMA_BASE_FILE_PATH}.cpp)
if (EXISTS ${SCHEMA_HEADER_FILE_NAME})
    message("Not re-generating meta schema source files")
else()
    file (WRITE ${SCHEMA_HEADER_FILE_NAME} "#include <map>\n#include <string>\n#include <nlohmann/json.hpp>\nnamespace nlohmann {\n    namespace json_schema {\n        const extern std::map<std::string,json> builtin_schema_map;\n    }\n}\n")
    file (WRITE ${SCHEMA_CPP_FILE_NAME} "#include \"${SCHEMA_BASE_FILE_NAME}.h\"\nnamespace nlohmann {\n    namespace json_schema {\n        const std::map<std::string,json> builtin_schema_map = {\n")
    file (GLOB META_SCHEMA_PATHS
        2019-09/meta/*.json
        2020-12/meta/*.json
    )
    LIST(APPEND META_SCHEMA_PATHS
        2019-09/schema.json
        2020-12/schema.json
    )
    message("META_SCHEMA_PATHS = ${META_SCHEMA_PATHS}")
    foreach(ext_json_path ${META_SCHEMA_PATHS})
        addSchemaFileToBuild(${ext_json_path} APPEND)  # subsequent invocations of addSchemaFileToBuild(), use APPEND
    endforeach()
    file (APPEND ${SCHEMA_CPP_FILE_NAME} "        };\n    }\n}\n")
endif()

target_sources(nlohmann_json_schema_validator PRIVATE ${SCHEMA_CPP_FILE_NAME})
target_include_directories(nlohmann_json_schema_validator PRIVATE ${CMAKE_BINARY_DIR}/json-schema-spec)

