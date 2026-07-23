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
using score::mw::health::deadline::testing_support::MockDeadline;
using score::mw::health::deadline::testing_support::MockDeadlineMonitor;
using score::mw::health::internal::FFICode;
using score::mw::health::internal::FFIHandle;
using score::mw::health::internal::kSuccess;
using score::mw::health::internal::non_null_handle_sentinel;

// Mock C FFI surface consumed by deadline_monitor.cpp, replacing the real Rust implementation.
// The builder-level functions are unused stubs: the mock chain never drives a real
// `DeadlineMonitorBuilder` (see MockHealthMonitor::as_health_monitor()/get_deadline_monitor() and
// MockDeadlineMonitor::as_deadline_monitor(), which build with zero sub-builders registered).
// `deadline_monitor_get_deadline` routes to the requested tag's `MockDeadline` (registered via
// `MockDeadlineMonitor::mock_deadline_for`), and `deadline_start`/`deadline_stop`/`deadline_destroy`
// route to that same `MockDeadline` via its own address, which is now the FFIHandle.
extern "C" {

FFICode deadline_monitor_builder_create(FFIHandle* deadline_monitor_builder_handle_out)
{
    // Must be non-null: DroppableFFIHandle treats a nullptr handle as "already dropped", and
    // HealthMonitorBuilder::add_deadline_monitor() asserts drop_by_rust() on this handle succeeds.
    // The value itself is never dereferenced -- this builder's handle is discarded once its tag
    // is registered on the owning MockHealthMonitor (see mock_health_monitor_ffi.cpp).
    *deadline_monitor_builder_handle_out = non_null_handle_sentinel();
    return static_cast<FFICode>(kSuccess);
}

FFICode deadline_monitor_builder_destroy([[maybe_unused]] FFIHandle deadline_monitor_builder_handle)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode deadline_monitor_builder_add_deadline(
    [[maybe_unused]] FFIHandle deadline_monitor_builder_handle,
    [[maybe_unused]] const DeadlineTag* deadline_tag,
    [[maybe_unused]] uint32_t min_ms,
    [[maybe_unused]] uint32_t max_ms)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode deadline_monitor_get_deadline(
    FFIHandle deadline_monitor_handle,
    const DeadlineTag* deadline_tag,
    FFIHandle* deadline_handle_out)
{
    auto* monitor_mock = reinterpret_cast<MockDeadlineMonitor*>(deadline_monitor_handle);
    auto* deadline_mock = monitor_mock->find_deadline(*deadline_tag);
    if (deadline_mock == nullptr)
    {
        return static_cast<FFICode>(Error::NotFound);
    }
    *deadline_handle_out = reinterpret_cast<FFIHandle>(deadline_mock);
    return static_cast<FFICode>(kSuccess);
}

FFICode deadline_monitor_destroy([[maybe_unused]] FFIHandle deadline_monitor_handle)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode deadline_destroy(FFIHandle deadline_handle)
{
    return reinterpret_cast<MockDeadline*>(deadline_handle)->destroy();
}

FFICode deadline_start(FFIHandle deadline_handle)
{
    return reinterpret_cast<MockDeadline*>(deadline_handle)->start();
}

FFICode deadline_stop(FFIHandle deadline_handle)
{
    return reinterpret_cast<MockDeadline*>(deadline_handle)->stop();
}

}  // extern "C"
