# *******************************************************************************
# Copyright (c) 2026 Contributors to the Eclipse Foundation
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

load("@rules_cc//cc:defs.bzl", "cc_library")
load("@score_baselibs//:bazel/unit_tests.bzl", "cc_gtest_unit_test")

cc_library(
    name = "mock",
    srcs = [
        "mock_deadline_ffi.cpp",
        "mock_health_monitor_ffi.cpp",
        "mock_heartbeat_logic_ffi.cpp",
        "mock_thread_ffi.cpp",
    ],
    hdrs = [
        "mock_deadline.h",
        "mock_deadline_monitor.h",
        "mock_health_monitor.h",
        "mock_heartbeat_monitor.h",
        "mock_logic_monitor.h",
        "testing_factory.h",
    ],
    include_prefix = "score/mw/health/mock",
    strip_include_prefix = "/score/health_monitor/src/cpp/mock",
    visibility = ["//visibility:public"],
    deps = [
        "//score/health_monitor/src/cpp:common",
        "//score/health_monitor/src/cpp:health_monitor",
        "//score/health_monitor/src/cpp/deadline:deadline_monitor",
        "//score/health_monitor/src/cpp/heartbeat:heartbeat_monitor",
        "//score/health_monitor/src/cpp/logic:logic_monitor",
        "@googletest//:gtest",
    ],
)

cc_gtest_unit_test(
    name = "unit_tests",
    srcs = [
        "tests/deadline_monitor_test.cpp",
        "tests/health_monitor_test.cpp",
        "tests/heartbeat_monitor_test.cpp",
        "tests/logic_monitor_test.cpp",
    ],
    deps = [
        ":mock",
        "//score/health_monitor/src/cpp:health_monitor",
    ],
)

cc_gtest_unit_test(
    name = "integration_tests",
    srcs = [
        "tests/example_component_test.cpp",
    ],
    deps = [
        ":mock",
        "//score/health_monitor/src/cpp:health_monitor",
    ],
)
