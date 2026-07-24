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
#include "mock_heartbeat_monitor.h"
#include <cstdint>

using score::mw::health::heartbeat::testing_support::MockHeartbeatMonitor;
using score::mw::health::internal::FFICode;
using score::mw::health::internal::FFIHandle;
using score::mw::health::internal::kSuccess;
using score::mw::health::internal::non_null_handle_sentinel;

// Mock C FFI surface consumed by heartbeat_monitor.cpp, replacing the real Rust implementation.
// The builder-level create function must return a non-null handle -- see
// mock_deadline_monitor_ffi.cpp's deadline_monitor_builder_create for why -- but the value is
// otherwise unused: HealthMonitorBuilder::add_heartbeat_monitor() discards it once the tag is
// registered on the owning MockHealthMonitor (see mock_health_monitor_ffi.cpp). The leaf function
// routes to the mock via its own address, which is the FFIHandle handed out at
// `health_monitor_get_heartbeat_monitor` or by the standalone
// `MockHeartbeatMonitor::as_heartbeat_monitor()`.
extern "C" {

FFICode heartbeat_monitor_builder_create(
    [[maybe_unused]] uint32_t range_min_ms,
    [[maybe_unused]] uint32_t range_max_ms,
    FFIHandle* heartbeat_monitor_builder_handle_out)
{
    *heartbeat_monitor_builder_handle_out = non_null_handle_sentinel();
    return static_cast<FFICode>(kSuccess);
}

FFICode heartbeat_monitor_builder_destroy([[maybe_unused]] FFIHandle heartbeat_monitor_builder_handle)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode heartbeat_monitor_destroy([[maybe_unused]] FFIHandle heartbeat_monitor_handle)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode heartbeat_monitor_heartbeat(FFIHandle heartbeat_monitor_handle)
{
    reinterpret_cast<MockHeartbeatMonitor*>(heartbeat_monitor_handle)->heartbeat();
    return static_cast<FFICode>(kSuccess);
}

}  // extern "C"
