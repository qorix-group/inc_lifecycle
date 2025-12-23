/********************************************************************************
 * Copyright (c) 2025 Contributors to the Eclipse Foundation
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
#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <expected>

enum class hm_Status : int32_t
{
    Running,
    Disabled,
    Failed,
};

enum class hm_Error : int32_t
{
    NoError,
    BadParameter,
    NotAllowed,
    OutOfMemory,
    Generic,
};


struct hm_HeartbeatMonitor;
extern "C"
{
    hm_HeartbeatMonitor *hm_hbm_new(const uint64_t min_heartbeat_cycle_ms, const uint64_t max_heartbeat_cycle_ms);
    hm_Error hm_hbm_enable(hm_HeartbeatMonitor *monitor);
    hm_Error hm_hbm_disable(hm_HeartbeatMonitor *monitor);
    std::expected<hm_Status, hm_Error>  hm_hbm_report_heartbeat(hm_HeartbeatMonitor *monitor);
    void hm_hbm_delete(hm_HeartbeatMonitor **monitor);
}
