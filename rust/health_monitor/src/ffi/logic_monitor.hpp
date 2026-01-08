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

#ifndef HM_LOGIC_MONITOR_H
#define HM_LOGIC_MONITOR_H

#include <cassert>
#include <expected>
#include "common.hpp"

struct hm_LogicMonitorBuilder;
struct hm_LogicMonitor;

struct hm_LogicMonitorState
{
    uint64_t hash;

    bool operator==(const hm_LogicMonitorState &other) const {
        return this->hash == other.hash;
    }
};

extern "C"
{
    hm_LogicMonitorState hm_lm_state_from_str(const char *name);
    hm_LogicMonitorBuilder *hm_lmb_new(hm_LogicMonitorState initial_state);
    void hm_lmb_delete(
        hm_LogicMonitorBuilder **builder);  // To be called only if hm_lmb_build wasn't called.
    void hm_lmb_add_transition(hm_LogicMonitorBuilder *builder, hm_LogicMonitorState from,
                               hm_LogicMonitorState to);
    hm_Error hm_lmb_build(hm_LogicMonitorBuilder **builder, hm_LogicMonitor **out);

    void hm_lm_delete(hm_LogicMonitor **monitor);
    hm_Error hm_lm_transition(hm_LogicMonitor *monitor, hm_LogicMonitorState to);
    hm_Error hm_lm_enable(hm_LogicMonitor *monitor);
    hm_Error hm_lm_disable(hm_LogicMonitor *monitor);
    hm_Status hm_lm_status(const hm_LogicMonitor *monitor);
    hm_LogicMonitorState hm_lm_state(const hm_LogicMonitor *monitor);
}

namespace hm {

struct LogicMonitorState : hm_LogicMonitorState
{
    LogicMonitorState(const char *name) : hm_LogicMonitorState(hm_lm_state_from_str(name)) {}
    LogicMonitorState(hm_LogicMonitorState &&state) : hm_LogicMonitorState(std::move(state)) {}

    bool operator==(const LogicMonitorState &other) const
    {
        return hm_LogicMonitorState::operator==(other);
    }
};

struct LogicMonitor
{
    LogicMonitor(hm_LogicMonitor *ptr) : ptr(ptr) {}

    LogicMonitor(const LogicMonitor &) = delete;
    LogicMonitor &operator=(const LogicMonitor &) = delete;

    LogicMonitor(LogicMonitor &&other) {
        this->ptr = other.ptr;
        other.ptr = nullptr;
    }

    LogicMonitor &operator=(LogicMonitor &&other) {
        this->ptr = other.ptr;
        other.ptr = nullptr;

        return *this;
    }

    ~LogicMonitor() {
        if (this->ptr) {
            hm_lm_delete(&this->ptr);
        }
    }

    Error transition(LogicMonitorState to) {
        return hm_lm_transition(this->ptr, to);
    }

    Error enable() {
        return hm_lm_enable(this->ptr);
    }

    Error disable() {
        return hm_lm_disable(this->ptr);
    }

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

    std::expected<LogicMonitor, Error> build()
    {
        assert(this->ptr);  // Ensure that build wasn't already called.

        hm_LogicMonitor *ffi_lm_ptr;
        if (auto error = hm_lmb_build(&this->ptr, &ffi_lm_ptr); error == Error::NoError) {
            return LogicMonitor(ffi_lm_ptr);
        } else {
            return std::unexpected(error);
        }
    }

    hm_LogicMonitorBuilder *ffi_ptr() { return this->ptr; }

    const hm_LogicMonitorBuilder *ffi_ptr() const { return this->ptr; }

private:
    hm_LogicMonitorBuilder *ptr;
};

}

#endif //HM_LOGIC_MONITOR_H
