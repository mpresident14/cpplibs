load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "http_structures",
    srcs = ["http_structures.cpp"],
    hdrs = ["http_structures.hpp"],
    deps = [
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "http_handler",
    hdrs = ["http_handler.hpp"],
    deps = [
        ":http_structures",
        "//src/webserver/json",
        "//src/webserver/json:json_parser",
    ],
)

cc_library(
    name = "http_server",
    srcs = [
        "http_server.cpp",
        "http_structures.cpp",
    ],
    hdrs = [
        "http_server.hpp",
        "http_structures.hpp",
    ],
    linkopts = ["-pthread"],
    deps = [
        ":http_handler",
        ":http_structures",
        "@boost//:container_hash",
    ],
)

cc_binary(
    name = "main",
    srcs = ["main.cpp"],
    data = glob(["testfiles/*"]),
    deps = [
        ":http_server",
        ":http_structures",
        "//src/webserver/json:json_parser",
    ],
)

sh_test(
    name = "http_server_test",
    size = "small",
    srcs = ["http_server_test.bash"],
    data = glob(["testfiles/*"]) + [":main"],
    deps = ["@bazel_tools//tools/bash/runfiles"],
)
