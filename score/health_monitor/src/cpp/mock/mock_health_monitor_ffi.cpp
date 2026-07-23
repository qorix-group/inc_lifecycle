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
#include "mock_health_monitor.h"
#include <cstdint>

using score::mw::health::Error;
using score::mw::health::MonitorTag;
using score::mw::health::internal::FFICode;
using score::mw::health::internal::FFIHandle;
using score::mw::health::internal::kSuccess;
using score::mw::health::internal::non_null_handle_sentinel;
using score::mw::health::testing_support::MockHealthMonitor;

// Mock C FFI surface consumed by health_monitor.cpp, replacing the real Rust implementation.
//
// `MockHealthMonitor::as_health_monitor()` and the standalone `As*Monitor()` helpers construct their
// real production counterparts directly (see internal::ConstructibleFrom), bypassing this FFI
// surface entirely. The builder functions are reached only when a real `HealthMonitorBuilder` is
// driven -- which, in mock tests, must be obtained from `MockHealthMonitorBuilder`, whose handle is
// a test-held `MockHealthMonitor`. `health_monitor_builder_create` therefore hands out only an
// inert sentinel (never allocates, never leaks); a `HealthMonitorBuilder` default-constructed
// against this FFI is unsupported. `_add_*_monitor`/`_build`/`_get_*_monitor` operate on whichever
// `MockHealthMonitor` handle they are given.
extern "C" {

FFICode health_monitor_builder_create(FFIHandle* health_monitor_builder_handle_out)
{
    *health_monitor_builder_handle_out = non_null_handle_sentinel();
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_builder_destroy([[maybe_unused]] FFIHandle health_monitor_builder_handle)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_builder_build(
    FFIHandle health_monitor_builder_handle,
    [[maybe_unused]] const uint64_t* supervisor_cycle_ms,
    [[maybe_unused]] const uint64_t* internal_cycle_ms,
    [[maybe_unused]] FFIHandle thread_parameters_handle,
    FFIHandle* health_monitor_handle_out)
{
    *health_monitor_handle_out = health_monitor_builder_handle;
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_builder_add_deadline_monitor(
    FFIHandle health_monitor_builder_handle,
    const MonitorTag* monitor_tag,
    [[maybe_unused]] FFIHandle deadline_monitor_builder_handle)
{
    reinterpret_cast<MockHealthMonitor*>(health_monitor_builder_handle)->mock_deadline_monitor_for(*monitor_tag);
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_builder_add_heartbeat_monitor(
    FFIHandle health_monitor_builder_handle,
    const MonitorTag* monitor_tag,
    [[maybe_unused]] FFIHandle heartbeat_monitor_builder_handle)
{
    reinterpret_cast<MockHealthMonitor*>(health_monitor_builder_handle)->mock_heartbeat_monitor_for(*monitor_tag);
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_builder_add_logic_monitor(
    FFIHandle health_monitor_builder_handle,
    const MonitorTag* monitor_tag,
    [[maybe_unused]] FFIHandle logic_monitor_builder_handle)
{
    reinterpret_cast<MockHealthMonitor*>(health_monitor_builder_handle)->mock_logic_monitor_for(*monitor_tag);
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_get_deadline_monitor(
    FFIHandle health_monitor_handle,
    const MonitorTag* monitor_tag,
    FFIHandle* deadline_monitor_handle_out)
{
    auto* health_monitor_mock = reinterpret_cast<MockHealthMonitor*>(health_monitor_handle);
    auto* deadline_monitor_mock = health_monitor_mock->find_deadline_monitor(*monitor_tag);
    if (deadline_monitor_mock == nullptr)
    {
        return static_cast<FFICode>(Error::NotFound);
    }
    *deadline_monitor_handle_out = reinterpret_cast<FFIHandle>(deadline_monitor_mock);
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_get_heartbeat_monitor(
    FFIHandle health_monitor_handle,
    const MonitorTag* monitor_tag,
    FFIHandle* heartbeat_monitor_handle_out)
{
    auto* health_monitor_mock = reinterpret_cast<MockHealthMonitor*>(health_monitor_handle);
    auto* heartbeat_monitor_mock = health_monitor_mock->find_heartbeat_monitor(*monitor_tag);
    if (heartbeat_monitor_mock == nullptr)
    {
        return static_cast<FFICode>(Error::NotFound);
    }
    *heartbeat_monitor_handle_out = reinterpret_cast<FFIHandle>(heartbeat_monitor_mock);
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_get_logic_monitor(
    FFIHandle health_monitor_handle,
    const MonitorTag* monitor_tag,
    FFIHandle* logic_monitor_handle_out)
{
    auto* health_monitor_mock = reinterpret_cast<MockHealthMonitor*>(health_monitor_handle);
    auto* logic_monitor_mock = health_monitor_mock->find_logic_monitor(*monitor_tag);
    if (logic_monitor_mock == nullptr)
    {
        return static_cast<FFICode>(Error::NotFound);
    }
    *logic_monitor_handle_out = reinterpret_cast<FFIHandle>(logic_monitor_mock);
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_start([[maybe_unused]] FFIHandle health_monitor_handle)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_destroy([[maybe_unused]] FFIHandle health_monitor_handle)
{
    return static_cast<FFICode>(kSuccess);
}

}  // extern "C"
