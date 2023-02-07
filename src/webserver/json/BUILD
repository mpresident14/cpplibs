load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "json",
    hdrs = ["json.hpp"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "json_parser",
    srcs = ["json_parser.cpp"],
    hdrs = ["json_parser.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":json",
        "//src/logger",
        "@boost//:algorithm",
        "@boost//:dynamic_bitset",
    ],
)

genrule(
    name = "generate_parser",
    srcs = ["json.pgen"],
    outs = [
        "json_parser.hpp",
        "json_parser.cpp",
    ],
    cmd_bash = "$(execpath //src/parsers/generator:parsegen) -g $(rootpath json.pgen) -d $(RULEDIR)/ -i src/webserver/json/ -n json_parser",
    message = "Generating JSON parser",
    tools = ["//src/parsers/generator:parsegen"],
)

cc_test(
    name = "json_parser_test",
    size = "small",
    srcs = ["json_parser_test.cpp"],
    data = glob(["testdata/*"]),
    deps = [
        ":json",
        ":json_parser",
        "//src/testing:unit_test",
    ],
)
