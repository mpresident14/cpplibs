load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "injector",
    hdrs = [
        "details.hpp",
        "inject_exception.hpp",
        "injector.hpp",
        "typing.hpp",
        "util.hpp",
    ],
    visibility = ["//visibility:public"],
    deps = [":binding_map"],
)

cc_library(
    name = "binding_map",
    hdrs = [
        "binding_map.hpp",
        "util.hpp",
    ],
    visibility = ["//visibility:public"],
)
