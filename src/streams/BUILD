load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "stream",
    hdrs = [
        "map_fn.hpp",
        "stream.hpp",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//src/misc:movable_fn",
        "//src/streams/operations",
        "//src/streams/util:ptr_iterator",
    ],
)

cc_binary(
    name = "main",
    srcs = ["main.cpp"],
    deps = [":stream"],
)
