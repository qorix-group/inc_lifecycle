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
#include "mock_deadline_monitor.h"
#include <cstdint>

using score::mw::health::DeadlineTag;
using score::mw::health::Error;
using score::mw::health::internal::FFICode;
using score::mw::health::internal::FFIHandle;
using score::mw::health::internal::kSuccess;
using score::mw::health::deadline::testing_support::MockDeadline;
using score::mw::health::deadline::testing_support::MockDeadlineMonitor;

// Mock C FFI surface consumed by deadline_monitor.cpp, replacing the real Rust implementation.
// The builder-level functions are unused stubs: the mock chain never drives a real
// `DeadlineMonitorBuilder` (see MockHealthMonitor::AsHealthMonitor()/GetDeadlineMonitor() and
// MockDeadlineMonitor::AsDeadlineMonitor(), which build with zero sub-builders registered).
// `deadline_monitor_get_deadline` routes to the requested tag's `MockDeadline` (registered via
// `MockDeadlineMonitor::MockDeadlineFor`), and `deadline_start`/`deadline_stop`/`deadline_destroy`
// route to that same `MockDeadline` via its own address, which is now the FFIHandle.
extern "C" {

FFICode deadline_monitor_builder_create(FFIHandle* handle_out)
{
    // Must be non-null: DroppableFFIHandle treats a nullptr handle as "already dropped", and
    // HealthMonitorBuilder::add_deadline_monitor() asserts drop_by_rust() on this handle succeeds.
    // The value itself is never dereferenced -- this builder's handle is discarded once its tag
    // is registered on the owning MockHealthMonitor (see mock_health_monitor_ffi.cpp).
    *handle_out = reinterpret_cast<FFIHandle>(1);
    return static_cast<FFICode>(kSuccess);
}

FFICode deadline_monitor_builder_destroy(FFIHandle)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode deadline_monitor_builder_add_deadline(FFIHandle, const DeadlineTag*, uint32_t, uint32_t)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode deadline_monitor_get_deadline(FFIHandle monitor_handle, const DeadlineTag* deadline_tag, FFIHandle* handle_out)
{
    auto* monitor_mock = reinterpret_cast<MockDeadlineMonitor*>(monitor_handle);
    auto* deadline_mock = monitor_mock->FindDeadline(*deadline_tag);
    if (deadline_mock == nullptr)
    {
        return static_cast<FFICode>(Error::NotFound);
    }
    *handle_out = reinterpret_cast<FFIHandle>(deadline_mock);
    return static_cast<FFICode>(kSuccess);
}

FFICode deadline_monitor_destroy(FFIHandle)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode deadline_destroy(FFIHandle handle)
{
    return reinterpret_cast<MockDeadline*>(handle)->Destroy();
}

FFICode deadline_start(FFIHandle handle)
{
    return reinterpret_cast<MockDeadline*>(handle)->Start();
}

FFICode deadline_stop(FFIHandle handle)
{
    return reinterpret_cast<MockDeadline*>(handle)->Stop();
}

}  // extern "C"
