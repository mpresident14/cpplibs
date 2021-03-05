load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
  name = "string_parser_test",
  srcs = ["string_parser_test.cpp"],
  deps = [
    "//src/testing:unit_test",
    "//src/parsers/combinator:parsers",
    "//src/parsers/combinator/testing:utils",
  ],
)
