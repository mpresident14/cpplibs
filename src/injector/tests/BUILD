load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
  name = "injector_test",
  srcs = ["injector_test.cpp"],
  deps = [
    "//src/testing:unit_test",
    "//src/injector",
  ],
)

cc_test(
  name = "binding_map_test",
  srcs = ["binding_map_test.cpp"],
  deps = [
    "//src/testing:unit_test",
    "//src/injector:binding_map",
  ],
)

cc_test(
  name = "module_test",
  srcs = ["module_test.cpp"],
  deps = [
    "//src/testing:unit_test",
    "//src/injector",
  ],
)
