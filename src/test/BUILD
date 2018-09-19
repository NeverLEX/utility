package(default_visibility = ["//visibility:public"])

cc_test(
    name = "dev_tools_test",
    srcs = [
        "dev-tools_test.cc",
    ],
    deps = [
        "//src/common:headers",
    ],
    timeout="short",
)

cc_test(
    name = "option_parser_test",
    srcs = [
        "option-parser_test.cc",
    ],
    deps = [
        "//src/common:headers",
    ],
    timeout="short",
)

cc_test(
    name = "defer_test",
    srcs = [
        "defer_test.cc",
    ],
    deps = [
        "//src/common:headers",
    ],
    timeout="short",
)

cc_test(
    name = "huffman_test",
    srcs = [
        "huffman_test.cc",
    ],
    deps = [
        "//src/common:headers",
        "//src/packer:huffman",
        "@com_google_googletest//:gtest",
    ],
    timeout="short",
)

cc_test(
    name = "packer_test",
    srcs = [
        "packer_test.cc",
    ],
    deps = [
        "//src/packer:packer",
        "@com_google_googletest//:gtest",
    ],
    data = [
        "//testdata:mytestdata",
    ],
    args = [
        "--data_path=./testdata/mytestdata/"
    ],
    timeout="short",
)

cc_test(
    name = "arraylist_test",
    srcs = [
        "arraylist_test.cc",
    ],
    deps = [
        "//src/common:headers",
    ],
    timeout="short",
)

cc_test(
    name = "arraymap_test",
    srcs = [
        "arraymap_test.cc",
    ],
    deps = [
        "//src/common:headers",
    ],
    timeout="short",
)

cc_test(
    name = "arraypool_test",
    srcs = [
        "arraypool_test.cc",
    ],
    deps = [
        "//src/common:headers",
    ],
    timeout="short",
)

cc_test(
    name = "topset_test",
    srcs = [
        "topset_test.cc",
    ],
    deps = [
        "//src/common:headers",
    ],
    timeout="short",
)

