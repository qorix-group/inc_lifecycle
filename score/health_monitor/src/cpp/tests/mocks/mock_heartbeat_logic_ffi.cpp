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
using score::mw::health::internal::FFICode;
using score::mw::health::internal::FFIHandle;
using score::mw::health::internal::kSuccess;
using score::mw::health::heartbeat::testing_support::MockHeartbeatMonitor;
using score::mw::health::logic::testing_support::MockLogicMonitor;

// Mock C FFI surface consumed by heartbeat_monitor.cpp/logic_monitor.cpp, replacing the real
// Rust implementation. The builder-level create functions must return a non-null handle -- see
// mock_deadline_ffi.cpp's deadline_monitor_builder_create for why -- but the value is otherwise
// unused: HealthMonitorBuilder::add_heartbeat_monitor/add_logic_monitor() discard it once the
// tag is registered on the owning MockHealthMonitor (see mock_health_monitor_ffi.cpp). The leaf
// functions route to the mock via its own address, which is the FFIHandle handed out at
// `health_monitor_get_heartbeat_monitor`/`health_monitor_get_logic_monitor`.
extern "C" {

FFICode heartbeat_monitor_builder_create(uint32_t, uint32_t, FFIHandle* handle_out)
{
    *handle_out = reinterpret_cast<FFIHandle>(1);
    return static_cast<FFICode>(kSuccess);
}

FFICode heartbeat_monitor_builder_destroy(FFIHandle)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode heartbeat_monitor_destroy(FFIHandle)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode heartbeat_monitor_heartbeat(FFIHandle handle)
{
    reinterpret_cast<MockHeartbeatMonitor*>(handle)->Heartbeat();
    return static_cast<FFICode>(kSuccess);
}

FFICode logic_monitor_builder_create(const StateTag*, FFIHandle* handle_out)
{
    *handle_out = reinterpret_cast<FFIHandle>(1);
    return static_cast<FFICode>(kSuccess);
}

FFICode logic_monitor_builder_destroy(FFIHandle)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode logic_monitor_builder_add_state(FFIHandle, const StateTag*, const StateTag*, size_t)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode logic_monitor_destroy(FFIHandle)
{
    return static_cast<FFICode>(kSuccess);
}

FFICode logic_monitor_transition(FFIHandle handle, const StateTag* target_state)
{
    auto result{reinterpret_cast<MockLogicMonitor*>(handle)->Transition(*target_state)};
    return result.has_value() ? static_cast<FFICode>(kSuccess) : static_cast<FFICode>(result.error());
}

FFICode logic_monitor_state(FFIHandle handle, StateTag* state_out)
{
    auto result{reinterpret_cast<MockLogicMonitor*>(handle)->State()};
    if (!result.has_value())
    {
        return static_cast<FFICode>(result.error());
    }
    *state_out = result.value();
    return static_cast<FFICode>(kSuccess);
}

}  // extern "C"
