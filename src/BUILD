load("@bazel_skylib//rules:common_settings.bzl", "string_flag")

string_flag(
    name = "nlohmann_version",
    # default nlohmann library version.
    build_setting_default = "3.11.3",
    values = [
        "3.11.2",
        "3.11.3",
    ],
)

config_setting(
    name = "nlohmann_v3.11.3",
    flag_values = {":nlohmann_version": "3.11.3" },
)

config_setting(
    name = "nlohmann_v3.11.2",
    flag_values = {":nlohmann_version": "3.11.2" },
)

cc_library(
    name = "json_schema_validator",
    hdrs = [
        "json-patch.hpp",
        "smtp-address-validator.hpp",
        "nlohmann/json-schema.hpp",
    ],
    includes = ["."],
    srcs = [
        "smtp-address-validator.cpp",
        "json-schema-draft7.json.cpp",
        "json-uri.cpp",
        "json-validator.cpp",
        "json-patch.cpp",
        "string-format-check.cpp",
    ],
    deps =
        select({
        "nlohmann_v3.11.3": ["@nlohmann_ext_3.11.3//:json", "@nlohmann_ext_3.11.3//:nlohmann_json_lib"],
        "nlohmann_v3.11.2": ["@nlohmann_ext_3.11.2//:json", "@nlohmann_ext_3.11.2//:nlohmann_json_lib"],
        }),
    visibility = ["//visibility:public"],
    alwayslink = True,
)
