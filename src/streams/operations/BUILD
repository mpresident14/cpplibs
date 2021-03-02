load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "operations",
    hdrs = [
        "distinct_op.hpp",
        "filter_op.hpp",
        "for_each_op.hpp",
        "operation.hpp",
    ],
    visibility = ["//src/streams:__subpackages__"],
    deps = [
        "//src/misc:movable_fn",
    ],
)
