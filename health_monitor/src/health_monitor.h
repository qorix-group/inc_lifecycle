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
#ifndef HM_HEALTH_MONITOR_H
#define HM_HEALTH_MONITOR_H

#include <cassert>
#include <chrono>

#include "ffi.h"
#include "ffi/common.hpp"
#include "ffi/deadline_monitor.hpp"

namespace hm
{

struct LogicMonitorState : hm_LogicMonitorState
{
    LogicMonitorState(const char *name) : hm_LogicMonitorState(hm_lm_state_from_str(name)) {}
    LogicMonitorState(hm_LogicMonitorState &&state) : hm_LogicMonitorState(std::move(state)) {}

    bool operator==(const LogicMonitorState &other)
    {
        return hm_LogicMonitorState::operator==(other);
    }
};

struct LogicMonitor
{
    LogicMonitor(hm_LogicMonitor *ptr) : ptr(ptr) {}

    LogicMonitor(const LogicMonitor &) = delete;
    LogicMonitor(LogicMonitor &&) = delete;
    LogicMonitor &operator=(const LogicMonitor &) = delete;
    LogicMonitor &operator=(LogicMonitor &&) = delete;

    ~LogicMonitor() { hm_lm_delete(&this->ptr); }

    void transition(LogicMonitorState to) { hm_lm_transition(this->ptr, to); }

    void enable() { hm_lm_enable(this->ptr); }

    void disable() { hm_lm_disable(this->ptr); }

    Status status() const { return hm_lm_status(this->ptr); }

    LogicMonitorState state() const { return hm_lm_state(this->ptr); }

    hm_LogicMonitor *ffi_ptr() { return this->ptr; }

    const hm_LogicMonitor *ffi_ptr() const { return this->ptr; }

private:
    hm_LogicMonitor *ptr;
};

struct LogicMonitorBuilder
{
    LogicMonitorBuilder(LogicMonitorState initial_state) : ptr(hm_lmb_new(initial_state)) {}

    LogicMonitorBuilder(const LogicMonitorBuilder &) = delete;
    LogicMonitorBuilder(LogicMonitorBuilder &&) = delete;
    LogicMonitorBuilder &operator=(const LogicMonitorBuilder &) = delete;
    LogicMonitorBuilder &operator=(LogicMonitorBuilder &&) = delete;

    ~LogicMonitorBuilder()
    {
        if (this->ptr)
        {
            hm_lmb_delete(&this->ptr);
        }
    }

    LogicMonitorBuilder &add_transition(LogicMonitorState from, LogicMonitorState to)
    {
        hm_lmb_add_transition(this->ptr, from, to);

        return *this;
    }

    LogicMonitor build()
    {
        assert(this->ptr);  // Ensure that build wasn't already called.
        return LogicMonitor(hm_lmb_build(&this->ptr));
    }

    hm_LogicMonitorBuilder *ffi_ptr() { return this->ptr; }

    const hm_LogicMonitorBuilder *ffi_ptr() const { return this->ptr; }

private:
    hm_LogicMonitorBuilder *ptr;
};

struct HeartbeatMonitor
{
    HeartbeatMonitor(hm_HeartbeatMonitor *ptr) : ptr(ptr) {}

    HeartbeatMonitor(const HeartbeatMonitor &) = delete;
    HeartbeatMonitor(HeartbeatMonitor &&) = delete;
    HeartbeatMonitor &operator=(const HeartbeatMonitor &) = delete;
    HeartbeatMonitor &operator=(HeartbeatMonitor &&) = delete;

    ~HeartbeatMonitor() { hm_hbm_delete(&this->ptr); }

    void enable() { hm_hbm_enable(this->ptr); }

    void disable() { hm_hbm_disable(this->ptr); }

    uint64_t heartbeat_cycle() { return hm_hbm_heartbeat_cycle(this->ptr); }

    uint64_t last_heartbeat() { return hm_hbm_last_heartbeat(this->ptr); }

    void send_heartbeat() { hm_hbm_send_heartbeat(this->ptr); }

    hm_HeartbeatMonitor *ffi_ptr() { return this->ptr; }

    const hm_HeartbeatMonitor *ffi_ptr() const { return this->ptr; }

private:
    hm_HeartbeatMonitor *ptr;
};

struct HealthMonitor
{
    HealthMonitor(const DeadlineMonitor &deadline_monitor, const LogicMonitor &logic_monitor,
                  const HeartbeatMonitor &heartbeat_monitor, const AliveMonitorFfi &alive_monitor,
                  std::chrono::milliseconds report_interval)
        : ptr(hm_new(deadline_monitor.ffi_ptr(), logic_monitor.ffi_ptr(),
                     heartbeat_monitor.ffi_ptr(), &alive_monitor, report_interval.count()))
    {
    }

    HealthMonitor(const LogicMonitorBuilder &) = delete;
    HealthMonitor(LogicMonitorBuilder &&) = delete;
    HealthMonitor &operator=(const LogicMonitorBuilder &) = delete;
    HealthMonitor &operator=(LogicMonitorBuilder &&) = delete;

    ~HealthMonitor() { delete (&this->ptr); }

    Status status() const { return hm_status(this->ptr); }

    hm_HealthMonitor *ffi_ptr() { return this->ptr; }

    const hm_HealthMonitor *ffi_ptr() const { return this->ptr; }

private:
    hm_HealthMonitor *ptr;
};

}  // namespace hm

#endif  // HM_HEALTH_MONITOR_H
