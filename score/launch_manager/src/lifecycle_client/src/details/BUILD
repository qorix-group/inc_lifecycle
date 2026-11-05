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
load("//tests/utils/bazel:unit_test.bzl", "lm_cc_test")

# Both libraries define the class in report_running_impl.hpp; //config:lifecycle_backend picks which one is linked.
alias(
    name = "report_running_impl",
    actual = select({
        "//config:lifecycle_backend_launch_manager": ":report_running_impl_launch_manager",
        "//config:lifecycle_backend_plms": ":report_running_impl_plms",
    }),
    visibility = ["//score/launch_manager/src/lifecycle_client:__pkg__"],
)

cc_library(
    name = "report_running_impl_launch_manager",
    srcs = ["report_running_impl.cpp"],
    hdrs = ["report_running_impl.hpp"],
    include_prefix = "score/mw/lifecycle/lifecycle_client/details",
    strip_include_prefix = "/score/launch_manager/src/lifecycle_client/src/details",
    visibility = ["//score/launch_manager/src/lifecycle_client:__pkg__"],
    deps = [
        "//score/launch_manager:error_event",
        "//score/launch_manager/src/daemon/src/common:constants",
        "//score/launch_manager/src/daemon/src/common:log",
        "//score/launch_manager/src/daemon/src/osal:ipc_comms",
        "@score_baselibs//score/result",
    ],
)

# Reports Running by attaching the name PLMS waits for with waitfor_attachname().
cc_library(
    name = "report_running_impl_plms",
    srcs = ["report_running_impl_plms.cpp"],
    hdrs = ["report_running_impl.hpp"],
    include_prefix = "score/mw/lifecycle/lifecycle_client/details",
    strip_include_prefix = "/score/launch_manager/src/lifecycle_client/src/details",
    target_compatible_with = ["@platforms//os:qnx"],
    visibility = ["//score/launch_manager/src/lifecycle_client:__pkg__"],
    deps = [
        "//score/launch_manager:error_event",
        "//score/launch_manager/src/daemon/src/common:log",
        "//score/launch_manager/src/daemon/src/osal:ipc_comms",
        "@score_baselibs//score/result",
    ],
)

# Links the PLMS library directly, so it tests that backend whatever //config:lifecycle_backend selects.
lm_cc_test(
    name = "report_running_impl_plms_UT",
    srcs = ["report_running_impl_plms_UT.cpp"],
    target_compatible_with = ["@platforms//os:qnx"],
    visibility = ["//visibility:public"],
    deps = [
        ":report_running_impl_plms",
        "//score/launch_manager:error_event",
        "@googletest//:gtest_main",
    ],
)
