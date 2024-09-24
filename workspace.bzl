load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# instructions to build nlohmann library after downloading it.
build_file_content_txt = """
cc_library(
    name = "nlohmann_json_lib",
    hdrs = glob([
        "include/nlohmann/*.hpp",
        "include/nlohmann/**/*.hpp",
        "include/nlohmann/*/*/*.hpp",
    ]),
    includes = ["nlohmann_json_lib"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "json",
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [":nlohmann_json_lib"],
)"""

# supported versions repositories
def load_nlohmann_3_11_3():
    http_archive(
        name = "nlohmann_ext_3.11.3",
        urls = ["https://github.com/nlohmann/json/archive/v3.11.3.tar.gz"],
        sha256 = "0d8ef5af7f9794e3263480193c491549b2ba6cc74bb018906202ada498a79406",
        build_file_content = build_file_content_txt,

        strip_prefix = "json-3.11.3",
    )

def load_nlohmann_3_11_2():
    http_archive(
        name = "nlohmann_ext_3.11.2",
        urls = ["https://github.com/nlohmann/json/archive/v3.11.2.tar.gz"],
        sha256 = "d69f9deb6a75e2580465c6c4c5111b89c4dc2fa94e3a85fcd2ffcd9a143d9273",
        build_file_content = build_file_content_txt,

        strip_prefix = "json-3.11.2",
    )
