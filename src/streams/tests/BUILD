load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "test_map",
    size = "small",
    srcs = ["test_map.cpp"],
    deps = [
        "//src/streams/testing",
        "//src/streams:stream",
        "//src/testing:unit_test",
    ],
)

cc_test(
    name = "test_filter",
    size = "small",
    srcs = ["test_filter.cpp"],
    deps = [
        "//src/streams/testing",
        "//src/streams:stream",
        "//src/testing:unit_test",
    ],
)

cc_test(
    name = "test_distinct",
    size = "small",
    srcs = ["test_distinct.cpp"],
    deps = [
        "//src/streams/testing",
        "//src/streams:stream",
        "//src/testing:unit_test",
    ],
)
