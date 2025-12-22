# *******************************************************************************
# Copyright (c) 2025 Contributors to the Eclipse Foundation
#
# See the NOTICE file(s) distributed with this work for additional
# information regarding copyright ownership.
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

load("@rules_rust//rust:defs.bzl", "rust_binary", "rust_library", "rust_static_library", "rust_test")

cc_library(
    name = "cpp_bindings",
    srcs = [
        "src/ffi/ffi.cpp",
    ],
    hdrs = glob(["src/ffi/*.h"]),
    cxxopts = [
        "-std=c++23",
    ],
    visibility = ["//visibility:public"],
)

rust_static_library(
    name = "health_monitoring_lib",
    srcs = glob([
        "src/common/*.rs",
        "src/heartbeat_monitor/*.rs",
        "src/ffi/*.rs",
    ]) + [
        "src/lib.rs",
    ],
    crate_name = "health_monitoring_lib",
    visibility = ["//visibility:public"],
    deps = [
        "@score_baselibs_rust//src/containers",
    ],
)

cc_binary(
    name = "heartbeat_monitor_example",
    srcs = [
        "examples/heartbeat_monitor.cpp",
    ],
    cxxopts = [
        "-std=c++23",
    ],
    includes = [
        "src/ffi",
    ],
    linkopts = [
        "-lpthread",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":cpp_bindings",
        ":health_monitoring_lib",
    ],
)

rust_test(
    name = "tests",
    crate = ":health_monitoring_lib",
    deps = [
        ":health_monitoring_lib",
    ],
)
