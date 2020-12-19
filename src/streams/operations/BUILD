load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
  name = "operations",
  hdrs = [
    "operation.hpp",
    "filter_op.hpp",
    "distinct_op.hpp",
  ],
  deps = [
    "//src/misc:movable_fn",
  ],
  visibility = ["//src/streams:__subpackages__"],
)
