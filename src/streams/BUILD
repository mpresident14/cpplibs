load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
  name = "stream",
  hdrs = [
    "stream.hpp",
    "typing.hpp",
    "map_fn.hpp",
  ],
  deps = [
    "//src/misc:movable_fn",
    "//src/streams/operations:operations",
  ],
  visibility = ["//visibility:public"],
)



cc_binary(
  name = "main",
  srcs = ["main.cpp"],
  deps = [":stream"]
)


cc_test(
  name = "test_map",
  srcs = ["test_map.cpp"],
  deps = [
    ":stream",
    "//src/testing:unit_test",
    "//src/streams/testing",
  ],
  size = "small",
)
