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
#include "mock_logic_monitor.h"
#include <cstddef>

using score::mw::health::StateTag;
using score::mw::health::internal::FFICode;
using score::mw::health::internal::FFIHandle;
using score::mw::health::internal::kSuccess;
using score::mw::health::internal::non_null_handle_sentinel;
using score::mw::health::logic::testing_support::MockLogicMonitor;

// Mock C FFI surface consumed by logic_monitor.cpp, replacing the real Rust implementation.
// The builder-level create function must return a non-null handle -- see
// mock_deadline_monitor_ffi.cpp's deadline_monitor_builder_create for why -- but the value is
// otherwise unused: HealthMonitorBuilder::add_logic_monitor() discards it once the tag is
// registered on the owning MockHealthMonitor (see mock_health_monitor_ffi.cpp). The leaf functions
// route to the mock via its own address, which is the FFIHandle handed out at
// `health_monitor_get_logic_monitor` or by the standalone `MockLogicMonitor::as_logic_monitor()`.
extern "C" {

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
