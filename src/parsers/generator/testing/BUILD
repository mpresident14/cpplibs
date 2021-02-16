load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(
    default_testonly = 1,
    default_visibility = ["//src/parsers/generator/tests:__subpackages__"],
)

cc_library(
    name = "nullable_grammar",
    hdrs = ["nullable_grammar.hpp"],
    deps = ["//src/parsers/generator:common"],
)

cc_library(
    name = "expr_grammar",
    hdrs = ["expr_grammar.hpp"],
    deps = ["//src/parsers/generator:common"],
)

cc_library(
    name = "expr",
    hdrs = ["expr.hpp"],
)

cc_binary(
    name = "write_lexer",
    srcs = [
        "write_lexer.cpp",
    ],
    deps = [
        "//src/parsers/generator:generate",
    ],
)

cc_binary(
    name = "write_parser",
    srcs = [
        "write_parser.cpp",
    ],
    deps = [
        ":expr",
        ":expr_grammar",
        "//src/logger",
        "//src/parsers/generator:common",
        "//src/parsers/generator:generate",
        "//src/parsers/generator:pgen_parse",
        "//src/testing:unit_test",
    ],
)

genrule(
    name = "generate_test_lexer",
    outs = [
        "test_lexer.hpp",
        "test_lexer.cpp",
    ],
    cmd_bash = "$(execpath :write_lexer) $(RULEDIR)/ src/parsers/generator/testing/ test_lexer",
    message = "Generating test_lexer",
    tools = [":write_lexer"],
)

genrule(
    name = "generate_test_parser",
    outs = [
        "test_parser.hpp",
        "test_parser.cpp",
    ],
    cmd_bash = "$(execpath :write_parser) $(RULEDIR)/ src/parsers/generator/testing/ test_parser bad_test_parser",
    message = "Generating test parsers",
    tools = [":write_parser"],
)

genrule(
    name = "generate_pgen_expr_parser",
    srcs = [
        "expr.pgen",
    ],
    outs = [
        "pgen_expr_parser.hpp",
        "pgen_expr_parser.cpp",
    ],
    cmd_bash = "$(execpath //src/parsers/generator:parsegen) -g $(rootpath expr.pgen) -d $(RULEDIR)/ -i src/parsers/generator/testing/ -n pgen_expr_parser",
    message = "Generating pgen_expr_parser",
    tools = ["//src/parsers/generator:parsegen"],
)
