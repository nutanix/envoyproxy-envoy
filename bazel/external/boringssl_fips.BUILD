licenses(["notice"])  # Apache 2

# Nutanix specific: Create separate targets for building static and
# dynamic libraries. Static library is used to statically linked to
# envoy binary, while the dynamic library is linked to the test
# binaries. This saves disk space when multiple unit tests are
# run together for asan and tsan jobs.

cc_library(
    name = "crypto_static",
    srcs = [
        "crypto/libcrypto.a",
    ],
    hdrs = glob(["boringssl/include/openssl/*.h"]),
    defines = ["BORINGSSL_FIPS"],
    includes = ["boringssl/include"],
)

cc_library(
    name = "ssl_static",
    srcs = [
        "ssl/libssl.a",
    ],
    hdrs = glob(["boringssl/include/openssl/*.h"]),
    includes = ["boringssl/include"],
    deps = [":crypto_static"],
)

cc_library(
    name = "crypto_dynamic",
    srcs = [
        "crypto/libcrypto.so",
    ],
    hdrs = glob(["boringssl/include/openssl/*.h"]),
    defines = ["BORINGSSL_FIPS"],
    includes = ["boringssl/include"],
)

cc_library(
    name = "ssl_dynamic",
    srcs = [
        "ssl/libssl.so",
    ],
    hdrs = glob(["boringssl/include/openssl/*.h"]),
    includes = ["boringssl/include"],
    deps = [":crypto_dynamic"],
)

alias(
    name = "ssl",
    actual = select({
        "@envoy//bazel:dynamic_link_tests": "@boringssl_fips//:ssl_dynamic",
        "//conditions:default": "@boringssl_fips//:ssl_static",
    }),
    visibility = ["//visibility:public"],
)

alias(
    name = "crypto",
    actual = select({
        "@envoy//bazel:dynamic_link_tests": "@boringssl_fips//:crypto_dynamic",
        "//conditions:default": "@boringssl_fips//:crypto_static",
    }),
    visibility = ["//visibility:public"],
)

genrule(
    name = "build",
    srcs = glob(["boringssl/**"]),
    outs = [
        "crypto/libcrypto.a",
        "crypto/libcrypto.so",
        "ssl/libssl.a",
        "ssl/libssl.so",
    ],
    cmd = "$(location {}) $(location crypto/libcrypto.a) $(location crypto/libcrypto.so) $(location ssl/libssl.a) $(location ssl/libssl.so)".format("@envoy//bazel/external:boringssl_fips.genrule_cmd"),
    tools = ["@envoy//bazel/external:boringssl_fips.genrule_cmd"],
)
