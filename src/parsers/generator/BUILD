load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//src/parsers/generator:__subpackages__"])

cc_library(
    name = "common",
    hdrs = [
        "rules.hpp",
        "utils.hpp",
    ],
    deps = [
        "//src/streams:stream",
    ],
)

cc_library(
    name = "dfa",
    hdrs = ["dfa.hpp"],
    deps = [
        ":common",
        "//src/misc:ostreamable",
    ],
)

cc_library(
    name = "null_first",
    srcs = ["null_first.cpp"],
    hdrs = ["null_first.hpp"],
    deps = [
        ":common",
        "@boost//:dynamic_bitset",
    ],
)

cc_library(
    name = "queue_set",
    hdrs = ["queue_set.hpp"],
)

cc_library(
    name = "build_parser",
    srcs = [
        "build_parser.cpp",
        "queue_set.hpp",
    ],
    hdrs = [
        "build_parser.hpp",
    ],
    linkopts = ["-pthread"],
    deps = [
        ":common",
        ":dfa",
        ":null_first",
        "//src/logger",
        "//src/misc:ostreamable",
        "@boost//:dynamic_bitset",
    ],
)

cc_library(
    name = "regex",
    srcs = [
        "regex.cpp",
        "regex.hpp",
        "regex_merge.cpp",
        "regex_parser.cpp",
    ],
    hdrs = [
        "regex_merge.hpp",
        "regex_parser.hpp",
    ],
    deps = [
        ":common",
        ":dfa",
        "//src/logger",
        "@boost//:dynamic_bitset",
    ],
)

cc_library(
    name = "generate",
    srcs = [
        "generate.cpp",
        "parse_info.hpp",
    ],
    hdrs = [
        "generate.hpp",
    ],
    deps = [
        ":build_parser",
        ":regex",
        "//src/misc:ostreamable",
    ],
)

cc_binary(
    name = "write_pgen_lexer",
    srcs = [
        "pgen_grammar.hpp",
        "write_pgen_lexer.cpp",
    ],
    deps = [
        ":common",
        ":generate",
        "//src/logger",
    ],
)

# Needed to add --host_copt="-std=c++20" to bazelrc
# (see https://docs.bazel.build/versions/master/guide.html#build-configurations-and-cross-compilation)
genrule(
    name = "generate_pgen_lexer",
    outs = [
        "pgen_lexer.hpp",
        "pgen_lexer.cpp",
    ],
    cmd_bash = "$(execpath :write_pgen_lexer) $(RULEDIR)/ src/parsers/generator/ pgen_lexer",
    message = "Generating pgen_lexer",
    tools = [
        ":write_pgen_lexer",
    ],
)

cc_library(
    name = "pgen_parse",
    srcs = [
        "pgen_parse.cpp",
        ":generate_pgen_lexer",
    ],
    hdrs = [
        "parse_info.hpp",
        "pgen_grammar.hpp",
        "pgen_parse.hpp",
    ],
    deps = [
        ":common",
        "//src/logger",
        "@boost//:dynamic_bitset",
    ],
)

cc_binary(
    name = "parsegen",
    srcs = [
        "parsegen.cpp",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":generate",
        ":pgen_parse",
        "//src/logger",
    ],
)
