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

load("@score_baselibs//score/language/safecpp:toolchain_features.bzl", "COMPILER_WARNING_FEATURES")
load("@rules_cc//cc:defs.bzl", "cc_library")

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "lifecycle_client_lib",
    srcs = [
        "src/aasapplicationcontainer.cpp",
        "src/application.cpp",
        "src/applicationcontext.cpp",
        "src/lifecyclemanager.cpp",
        "src/runapplication.cpp",
    ],
    hdrs = [
        "include/aasapplicationcontainer.h",
        "include/application.h",
        "include/applicationcontext.h",
        "include/lifecyclemanager.h",
        "include/runapplication.h",
        ],
    copts = select({
        "//config:x86_64-qnx": [],
        "//conditions:default": ["-pthread"],
    }),
    linkopts = select({
        "//config:x86_64-qnx": [],
        "//conditions:default": ["-lpthread"],
    }),
    visibility = ["//visibility:public"],
    deps = [
        "@score_baselibs//score/memory:string_literal",
        "@score_baselibs//score/language/futurecpp",
        "@score_baselibs//score/os/utils:signal",
        "@score_baselibs//score/mw/log",
        "@score_baselibs//score/os:stdlib",
    ],
)

cc_library(
    name = "applicationcontext",
    srcs = [
        "src/applicationcontext.cpp",
    ],
    hdrs = [
        "include/applicationcontext.h",
    ],
    features = COMPILER_WARNING_FEATURES,
    tags = ["FUSA"],
    deps = [
        "@score_baselibs//score/memory:string_literal",
        "@score_baselibs//score/os:stdlib",
        "@score_baselibs//score/os/utils:signal",
        "@score_baselibs//score/mw/log",
    ],
)

cc_library(
    name = "application",
    srcs = [
        "src/application.cpp",
    ],
    hdrs = [
        "include/application.h",
    ],
    features = COMPILER_WARNING_FEATURES,
    tags = ["FUSA"],
    deps = [
        ":applicationcontext",
        "@score_baselibs//score/language/futurecpp",
    ],
)

cc_library(
    name = "aasapplicationcontainer",
    srcs = [
        "src/aasapplicationcontainer.cpp",
    ],
    hdrs = [
        "include/aasapplicationcontainer.h",
        "include/application.h",
        "include/lifecyclemanager.h"
    ],
    features = COMPILER_WARNING_FEATURES,
    deps = [
        ":applicationcontext",
    ],
)



cc_library(
    name = "lifecyclemanager",
    srcs = [
        "src/lifecyclemanager.cpp",
    ],
    hdrs = [
        "include/lifecyclemanager.h",
        "include/application.h",
    ],
    features = COMPILER_WARNING_FEATURES,
    tags = ["FUSA"],
    deps = [
        ":application",
        "@score_baselibs//score/os:stdlib",
        "@score_baselibs//score/os/utils:signal",
        "@score_baselibs//score/mw/log",
    ],
)

[
    cc_library(
        name = name,
        srcs = [
            "src/runapplication.cpp",
        ],
        hdrs = [
            "include/runapplication.h",
            "include/lifecyclemanager.h",
            "include/application.h"
        ],
        features = COMPILER_WARNING_FEATURES,
        tags = ["FUSA"],
        deps = [
            ":applicationcontext",
        ] + dependencies,
    )
    for name, dependencies in [
        (
            "lifecycle",
            [
            ],
        ),
        (
            "lifecycle_non_adaptive",
            [
            ],
        ),
    ]
]

filegroup(
    name = "all_sources",
    srcs = glob([
        "src/*.cpp",
        "include/*.h",
    ]),
)
