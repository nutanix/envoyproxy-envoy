load(
    "//bazel:envoy_build_system.bzl",
    "envoy_package",
)
load(
    "//test/extensions:extensions_build_system.bzl",
    "envoy_extension_cc_test",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_extension_cc_test(
    name = "reverse_connection_manager_test",
    srcs = ["reverse_connection_manager_test.cc"],
    extension_names = ["envoy.bootstrap.internal_listener"],
    deps = [
        "//source/extensions/bootstrap/reverse_connection:reverse_connection_includes",
        "//source/extensions/bootstrap/reverse_connection:reverse_connection_initiator_lib",
        "//test/mocks/event:event_mocks",
        "//test/mocks/network:network_mocks",
        "//test/mocks/stats:stats_mocks",
        "//test/test_common:test_runtime_lib",
    ],
)
