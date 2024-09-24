def validator_test(name) :
  native.cc_test(
    name = name,
    srcs = [name + ".cpp"],
    copts = [],
    deps = ["//src:json_schema_validator"])
