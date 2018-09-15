package(default_visibility = ["//visibility:public"])

cc_library(
    name = "huffman",
    hdrs = [
        "huffman.h",
    ],
    srcs = [
        "huffman.cc",
    ],
    deps = [
        "//src/common:headers"
    ],
    includes = ["./"],
    linkstatic = True,
)

cc_library(
    name = "packer",
    hdrs = [
        "packer.h",
    ],
    srcs = [
        "packer.cc",
    ],
    deps = [
        ":huffman",
    ],
    includes = ["./"],
    linkstatic = True,
)

cc_binary(
    name = "resource_packer",
    srcs = [
        "resource-packer.cc",
    ],
    deps = [
        ":packer",
    ],
)


