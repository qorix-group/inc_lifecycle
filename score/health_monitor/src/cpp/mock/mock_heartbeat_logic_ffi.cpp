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
#include "mock_logic_monitor.h"
#include <cstdint>

using score::mw::health::StateTag;
using score::mw::health::heartbeat::testing_support::MockHeartbeatMonitor;
using score::mw::health::internal::FFICode;
using score::mw::health::internal::FFIHandle;
using score::mw::health::internal::kSuccess;
using score::mw::health::internal::non_null_handle_sentinel;
using score::mw::health::logic::testing_support::MockLogicMonitor;

// Mock C FFI surface consumed by heartbeat_monitor.cpp/logic_monitor.cpp, replacing the real
// Rust implementation. The builder-level create functions must return a non-null handle -- see
// mock_deadline_ffi.cpp's deadline_monitor_builder_create for why -- but the value is otherwise
// unused: HealthMonitorBuilder::add_heartbeat_monitor/add_logic_monitor() discard it once the
// tag is registered on the owning MockHealthMonitor (see mock_health_monitor_ffi.cpp). The leaf
// functions route to the mock via its own address, which is the FFIHandle handed out at
// `health_monitor_get_heartbeat_monitor`/`health_monitor_get_logic_monitor` or by the standalone
// `MockHeartbeatMonitor::as_heartbeat_monitor()`/`MockLogicMonitor::as_logic_monitor()`.
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

FFICode logic_monitor_builder_create(
    [[maybe_unused]] const StateTag* initial_state,
    FFIHandle* logic_monitor_builder_handle_out)
{
    *logic_monitor_builder_handle_out = non_null_handle_sentinel();
    return static_cast<FFICode>(kSuccess);
}

FFICode logic_monitor_builder_destroy([[maybe_unused]] FFIHandle logic_monitor_builder_handle)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode logic_monitor_builder_add_state(
    [[maybe_unused]] FFIHandle logic_monitor_builder_handle,
    [[maybe_unused]] const StateTag* state,
    [[maybe_unused]] const StateTag* allowed_states,
    [[maybe_unused]] size_t num_allowed_states)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode logic_monitor_destroy([[maybe_unused]] FFIHandle logic_monitor_handle)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode logic_monitor_transition(FFIHandle logic_monitor_handle, const StateTag* target_state)
{
    auto result{reinterpret_cast<MockLogicMonitor*>(logic_monitor_handle)->transition(*target_state)};
    return result.has_value() ? static_cast<FFICode>(kSuccess) : static_cast<FFICode>(result.error());
}

FFICode logic_monitor_state(FFIHandle logic_monitor_handle, StateTag* state_out)
{
    auto result{reinterpret_cast<MockLogicMonitor*>(logic_monitor_handle)->state()};
    if (!result.has_value())
    {
        return static_cast<FFICode>(result.error());
    }
    *state_out = result.value();
    return static_cast<FFICode>(kSuccess);
}

}  // extern "C"
