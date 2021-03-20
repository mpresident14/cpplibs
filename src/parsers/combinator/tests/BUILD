load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "string_parser_test",
    srcs = ["string_parser_test.cpp"],
    deps = [
        "//src/parsers/combinator:parsers",
        "//src/parsers/combinator/testing:utils",
        "//src/testing:unit_test",
    ],
)

cc_test(
    name = "num_parser_test",
    srcs = ["num_parser_test.cpp"],
    deps = [
        "//src/parsers/combinator:parsers",
        "//src/parsers/combinator/testing:utils",
        "//src/testing:unit_test",
    ],
)

cc_test(
    name = "sequence_parser_test",
    srcs = ["sequence_parser_test.cpp"],
    deps = [
        "//src/parsers/combinator:parsers",
        "//src/parsers/combinator/testing:utils",
        "//src/testing:unit_test",
        "//src/testing:widget",
    ],
)

cc_test(
    name = "alt_parser_test",
    srcs = ["alt_parser_test.cpp"],
    deps = [
        "//src/parsers/combinator:parsers",
        "//src/parsers/combinator/testing:utils",
        "//src/testing:unit_test",
    ],
)

cc_test(
    name = "map_parser_test",
    srcs = ["map_parser_test.cpp"],
    deps = [
        "//src/parsers/combinator:parsers",
        "//src/parsers/combinator/testing:utils",
        "//src/testing:unit_test",
    ],
)

cc_test(
    name = "map_tuple_parser_test",
    srcs = ["map_tuple_parser_test.cpp"],
    deps = [
        "//src/parsers/combinator:parsers",
        "//src/parsers/combinator/testing:utils",
        "//src/testing:unit_test",
    ],
)

cc_test(
    name = "parse_test",
    srcs = ["parse_test.cpp"],
    deps = [
        "//src/parsers/combinator:parsers",
        "//src/testing:unit_test",
    ],
)
