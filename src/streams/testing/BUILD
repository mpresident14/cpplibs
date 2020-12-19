load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "testing",
    hdrs = [
        "widget.hpp",
    ],
    visibility = ["//src/streams:__subpackages__"],
)

cc_test(
    name = "test_map",
    size = "small",
    srcs = ["test_map.cpp"],
    deps = [
        ":testing",
        "//src/streams:stream",
        "//src/testing:unit_test",
    ],
)

cc_test(
    name = "test_filter",
    size = "small",
    srcs = ["test_filter.cpp"],
    deps = [
        ":testing",
        "//src/streams:stream",
        "//src/testing:unit_test",
    ],
)

cc_test(
    name = "test_distinct",
    size = "small",
    srcs = ["test_distinct.cpp"],
    deps = [
        ":testing",
        "//src/streams:stream",
        "//src/testing:unit_test",
    ],
)
