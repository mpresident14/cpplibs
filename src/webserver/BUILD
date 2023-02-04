load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

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
        "//src/misc:ostreamable",
    ],
)

cc_binary(
    name = "main",
    srcs = ["main.cpp"],
    deps = [":http_server"],
)
