load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//src/parsers/combinator:__subpackages__"])

cc_library(
    name = "parsers",
    hdrs = [
        "alt_parser.hpp",
        "execution_log.hpp",
        "map_parser.hpp",
        "more_type_traits.hpp",
        "num_parser.hpp",
        "parser.hpp",
        "parser_builder.hpp",
        "parsers.hpp",
        "sequence_parser.hpp",
        "string_parser.hpp",
    ],
    deps = [
        "//src/misc:more_type_traits",
    ],
)
