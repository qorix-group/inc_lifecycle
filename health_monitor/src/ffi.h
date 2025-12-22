// Copyright (c) 2025 Contributors to the Eclipse Foundation
//
// See the NOTICE file(s) distributed with this work for additional
// information regarding copyright ownership.
//
// This program and the accompanying materials are made available under the
// terms of the Apache License Version 2.0 which is available at
// <https://www.apache.org/licenses/LICENSE-2.0>
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef HM_FFI_H
#define HM_FFI_H

#include <cstdint>
#include "../alive_monitor/include/alive_monitor_ffi.h"
#include "ffi/common.hpp"
#include "ffi/deadline_monitor.hpp"

struct hm_LogicMonitorBuilder;
struct hm_LogicMonitor;

struct hm_LogicMonitorState
{
    uint64_t hash;

    bool operator==(const hm_LogicMonitorState &other) const { return this->hash == other.hash; }
};

typedef void hm_LogicMonitorOnStatusChanged(void *data, hm_Status from, hm_Status to);
typedef void hm_LogicMonitorOnStateChanged(void *data, hm_LogicMonitorState from,
                                           hm_LogicMonitorState to);

extern "C"
{
    hm_LogicMonitorState hm_lm_state_from_str(const char *name);
    hm_LogicMonitorBuilder *hm_lmb_new(hm_LogicMonitorState initial_state);
    void hm_lmb_delete(
        hm_LogicMonitorBuilder **builder);  // To be called only if hm_lmb_build wasn't called.
    void hm_lmb_add_transition(hm_LogicMonitorBuilder *builder, hm_LogicMonitorState from,
                               hm_LogicMonitorState to);
    void hm_lmb_add_hook(hm_LogicMonitorBuilder *builder,
                         hm_LogicMonitorOnStatusChanged on_status_changed,
                         void *on_status_changed_data,
                         hm_LogicMonitorOnStateChanged on_state_changed,
                         void *on_state_changed_data);
    hm_LogicMonitor *hm_lmb_build(hm_LogicMonitorBuilder **builder);

    void hm_lm_delete(hm_LogicMonitor **monitor);
    void hm_lm_transition(hm_LogicMonitor *monitor, hm_LogicMonitorState to);
    void hm_lm_enable(hm_LogicMonitor *monitor);
    void hm_lm_disable(hm_LogicMonitor *monitor);
    hm_Status hm_lm_status(const hm_LogicMonitor *monitor);
    hm_LogicMonitorState hm_lm_state(const hm_LogicMonitor *monitor);
}

struct hm_HeartbeatMonitor;

extern "C"
{
    hm_HeartbeatMonitor *hm_hbm_new(const uint64_t maximum_heartbeat_cycle_ms);
    void hm_hbm_enable(hm_HeartbeatMonitor *monitor);
    void hm_hbm_disable(hm_HeartbeatMonitor *monitor);
    void hm_hbm_send_heartbeat(hm_HeartbeatMonitor *monitor);
    void hm_hbm_delete(hm_HeartbeatMonitor **monitor);
    uint64_t hm_hbm_heartbeat_cycle(hm_HeartbeatMonitor *monitor);
    uint64_t hm_hbm_last_heartbeat(hm_HeartbeatMonitor *monitor);
}

struct hm_HealthMonitor;

extern "C"
{
    hm_HealthMonitor *hm_new(const hm_DeadlineMonitor *deadline_monitor,
                             const hm_LogicMonitor *logic_monitor,
                             const hm_HeartbeatMonitor *heartbeat_monitor,
                             const AliveMonitorFfi *alive_monitor, uint64_t report_interval_ms);
    void hm_delete(hm_HealthMonitor **monitor);
    hm_Status hm_status(const hm_HealthMonitor *monitor);
}

#endif  // HM_FFI_H
