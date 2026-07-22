/********************************************************************************
 * Copyright (c) 2026 Contributors to the Eclipse Foundation
 *
 * See the NOTICE file(s) distributed with this work for additional
 * information regarding copyright ownership.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#include "score/mw/health/thread.h"
#include <cstdint>

using score::mw::health::SchedulerPolicy;
using score::mw::health::internal::FFICode;
using score::mw::health::internal::FFIHandle;
using score::mw::health::internal::kSuccess;

// Linking the real `//score/health_monitor/src/cpp:health_monitor` aggregate library pulls in
// thread.cpp as a separate shared object, and this toolchain requires every symbol it references
// to be resolved even when unused at runtime. These are unreachable-in-practice stubs -- nothing
// in these tests constructs a `ThreadParameters`.
extern "C" {

FFICode scheduler_policy_priority_min(SchedulerPolicy, int32_t* priority_out)
{
    *priority_out = 0;
    return static_cast<FFICode>(kSuccess);
}

FFICode scheduler_policy_priority_max(SchedulerPolicy, int32_t* priority_out)
{
    *priority_out = 0;
    return static_cast<FFICode>(kSuccess);
}

FFICode thread_parameters_create(FFIHandle* handle_out)
{
    // Non-null: DroppableFFIHandle treats a nullptr handle as "already dropped" (see
    // mock_deadline_ffi.cpp's deadline_monitor_builder_create for the full explanation).
    *handle_out = reinterpret_cast<FFIHandle>(1);
    return static_cast<FFICode>(kSuccess);
}

FFICode thread_parameters_destroy(FFIHandle)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode thread_parameters_scheduler_parameters(FFIHandle, SchedulerPolicy, int32_t)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode thread_parameters_affinity(FFIHandle, const size_t*, size_t)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode thread_parameters_stack_size(FFIHandle, size_t)
{
    return static_cast<FFICode>(kSuccess);
}

}  // extern "C"
