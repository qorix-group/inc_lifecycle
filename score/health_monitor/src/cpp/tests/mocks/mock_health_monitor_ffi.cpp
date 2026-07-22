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
using score::mw::health::testing_support::MockHealthMonitor;

// Mock C FFI surface consumed by health_monitor.cpp, replacing the real Rust implementation.
//
// `MockHealthMonitor::AsHealthMonitor()`/`MockDeadlineMonitor::AsDeadlineMonitor()` construct
// their real production counterparts directly (see internal::ConstructibleFrom in common.h),
// bypassing this FFI surface entirely. It only remains load-bearing for code that constructs a
// real `HealthMonitorBuilder` directly: `health_monitor_builder_create` then allocates a fresh,
// otherwise-unreachable `MockHealthMonitor` behind it, and `health_monitor_get_deadline_monitor`/
// `_get_heartbeat_monitor`/`_get_logic_monitor` do a plain tag lookup against whatever
// `MockHealthMonitor` handle they're given.
extern "C" {

FFICode health_monitor_builder_create(FFIHandle* handle_out)
{
    *handle_out = reinterpret_cast<FFIHandle>(new MockHealthMonitor());
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_builder_destroy(FFIHandle)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_builder_build(FFIHandle health_monitor_builder_handle,
                                     const uint64_t*,
                                     const uint64_t*,
                                     FFIHandle,
                                     FFIHandle* handle_out)
{
    *handle_out = health_monitor_builder_handle;
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_builder_add_deadline_monitor(FFIHandle health_monitor_builder_handle,
                                                    const MonitorTag* monitor_tag,
                                                    FFIHandle)
{
    reinterpret_cast<MockHealthMonitor*>(health_monitor_builder_handle)->MockDeadlineMonitorFor(*monitor_tag);
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_builder_add_heartbeat_monitor(FFIHandle health_monitor_builder_handle,
                                                     const MonitorTag* monitor_tag,
                                                     FFIHandle)
{
    reinterpret_cast<MockHealthMonitor*>(health_monitor_builder_handle)->MockHeartbeatMonitorFor(*monitor_tag);
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_builder_add_logic_monitor(FFIHandle health_monitor_builder_handle,
                                                 const MonitorTag* monitor_tag,
                                                 FFIHandle)
{
    reinterpret_cast<MockHealthMonitor*>(health_monitor_builder_handle)->MockLogicMonitorFor(*monitor_tag);
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_get_deadline_monitor(FFIHandle health_monitor_handle,
                                            const MonitorTag* monitor_tag,
                                            FFIHandle* handle_out)
{
    auto* health_monitor_mock = reinterpret_cast<MockHealthMonitor*>(health_monitor_handle);
    auto* deadline_monitor_mock = health_monitor_mock->FindDeadlineMonitor(*monitor_tag);
    if (deadline_monitor_mock == nullptr)
    {
        return static_cast<FFICode>(Error::NotFound);
    }
    *handle_out = reinterpret_cast<FFIHandle>(deadline_monitor_mock);
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_get_heartbeat_monitor(FFIHandle health_monitor_handle,
                                             const MonitorTag* monitor_tag,
                                             FFIHandle* handle_out)
{
    auto* health_monitor_mock = reinterpret_cast<MockHealthMonitor*>(health_monitor_handle);
    auto* heartbeat_monitor_mock = health_monitor_mock->FindHeartbeatMonitor(*monitor_tag);
    if (heartbeat_monitor_mock == nullptr)
    {
        return static_cast<FFICode>(Error::NotFound);
    }
    *handle_out = reinterpret_cast<FFIHandle>(heartbeat_monitor_mock);
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_get_logic_monitor(FFIHandle health_monitor_handle,
                                         const MonitorTag* monitor_tag,
                                         FFIHandle* handle_out)
{
    auto* health_monitor_mock = reinterpret_cast<MockHealthMonitor*>(health_monitor_handle);
    auto* logic_monitor_mock = health_monitor_mock->FindLogicMonitor(*monitor_tag);
    if (logic_monitor_mock == nullptr)
    {
        return static_cast<FFICode>(Error::NotFound);
    }
    *handle_out = reinterpret_cast<FFIHandle>(logic_monitor_mock);
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_start(FFIHandle)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode health_monitor_destroy(FFIHandle)
{
    return static_cast<FFICode>(kSuccess);
}

}  // extern "C"
