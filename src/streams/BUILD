load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "stream",
    hdrs = [
        "map_fn.hpp",
        "stream.hpp",
        "more_type_traits.hpp",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//src/misc:movable_fn",
        "//src/streams/operations",
    ],
)

cc_binary(
    name = "main",
    srcs = ["main.cpp"],
    deps = [
        ":stream",
        "//src/misc:print_stuff",
    ],
)
