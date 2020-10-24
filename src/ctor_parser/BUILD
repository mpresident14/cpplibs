load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
  name = "ctor_parser",
  srcs = ["ctor_parser.cpp"],
  hdrs = ["ctor_parser.hpp"],
  visibility = ["//visibility:public"],
)

cc_binary(
  name = "ctor_parser_test",
  srcs = ["ctor_parser_test.cpp"],
  deps = [":ctor_parser"],
)
