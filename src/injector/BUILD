load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary")

cc_library(
  name = "injector",
  hdrs = [
    "injector.hpp",
    "details.hpp",
    "typing.hpp",
    "inject_exception.hpp",
  ],
  visibility = ["//visibility:public"],
)

cc_binary(
  name = "injector_test",
  srcs = ["injector_test.cpp"],
  deps = [
    "//src/testing:unit_test",
    "//src/injector",
  ],
)
