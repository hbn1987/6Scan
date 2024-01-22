cc_library(
    name = "utils",
    srcs = ["utils.cc"],
    hdrs= ["utils.h"],
    deps = [
        "@boost//:filesystem",
        "//external:glog",
        "//trace6:address",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
    ],
)

cc_binary(
    name = "route_path",
    srcs = ["route_path.cc"],
    deps = [
        "//external:glog",
        "//external:gflags",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse", 
        "//trace6:dump_result",
        "//trace6:address",
        ":utils",
        "//trace6:utils",
    ],
)