load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary")

cc_library(
  name = "injector",
  hdrs = [
    "injector.hpp",
    "details.hpp",
    "typing.hpp",
    "inject_exception.hpp",
    "util.hpp",
  ],
  deps = [":binding_map"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "binding_map",
  hdrs = [
    "binding_map.hpp",
    "util.hpp",
  ],
  visibility = ["//visibility:public"],
)

cc_binary(
  name = "injector_test",
  srcs = ["injector_test.cpp"],
  deps = [
    "//src/testing:unit_test",
    ":injector",
  ],
)

cc_binary(
  name = "binding_map_test",
  srcs = ["binding_map_test.cpp"],
  deps = [
    "//src/testing:unit_test",
    ":binding_map",
  ],
)
