load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "dfa_test",
    size = "small",
    srcs = ["dfa_test.cpp"],
    deps = ["//src/parsers/generator:dfa"],
)

cc_test(
    name = "null_first_test",
    size = "small",
    srcs = ["null_first_test.cpp"],
    deps = [
        "//src/parsers/generator:null_first",
        "//src/parsers/generator/testing:nullable_grammar",
    ],
)

cc_test(
    name = "queue_set_test",
    size = "small",
    srcs = ["queue_set_test.cpp"],
    deps = [
        "//src/parsers/generator:queue_set",
    ],
)

cc_test(
    name = "build_parser_test",
    size = "small",
    srcs = ["build_parser_test.cpp"],
    deps = [
        "//src/parsers/generator:build_parser",
        "//src/parsers/generator/testing:expr_grammar",
    ],
)

cc_test(
    name = "regex_test",
    size = "small",
    srcs = ["regex_test.cpp"],
    deps = [
        "//src/parsers/generator:regex",
    ],
)

cc_test(
    name = "tokenize_test",
    size = "small",
    srcs = [
        "tokenize_test.cpp",
        "//src/parsers/generator/testing:generate_test_lexer",
    ],
)

cc_test(
    name = "parse_test",
    size = "small",
    srcs = [
        "parse_test.cpp",
        "//src/parsers/generator/testing:generate_test_parser",
    ],
    deps = [
        "//src/parsers/generator/testing:expr",
        "@boost//:dynamic_bitset",
    ],
)

cc_test(
    name = "parsegen_test",
    size = "small",
    srcs = [
        "parsegen_test.cpp",
        "//src/parsers/generator/testing:generate_pgen_expr_parser",
    ],
    deps = [
        "//src/parsers/generator:parsegen",
        "//src/parsers/generator/testing:expr",
    ],
)
