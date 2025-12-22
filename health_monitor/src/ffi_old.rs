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
use crate::common::Status;
use crate::deadline_monitor::*;
use crate::health_monitor::*;
use crate::heartbeat_monitor::HeartbeatMonitor;
use crate::logic_monitor::*;
use alive_monitor::ffi::*;
use std::{
    ffi::{CStr, c_char},
    os::raw::c_void,
    ptr::null_mut,
    time::Duration,
};

//
// LogicMonitorBuilder
//

#[unsafe(no_mangle)]
extern "C" fn hm_lm_state_from_str(name: *const c_char) -> State {
    let name = unsafe { CStr::from_ptr(name) };

    // TODO: Propagate error.
    match name.to_str() {
        Ok(name) => State::from_str(name),
        Err(_err) => todo!("Report error")
    }
}

#[unsafe(no_mangle)]
extern "C" fn hm_lmb_new(initial_state: State) -> *mut LogicMonitorBuilder {
    Box::into_raw(Box::new(LogicMonitorBuilder::new(initial_state)))
}

#[unsafe(no_mangle)]
extern "C" fn hm_lmb_delete(builder_ptr: *mut *mut LogicMonitorBuilder) {
    let _builder = unsafe { Box::from_raw(*builder_ptr) };
    unsafe {
        *builder_ptr = null_mut();
    }
}

#[unsafe(no_mangle)]
extern "C" fn hm_lmb_add_transition(builder: *mut LogicMonitorBuilder, from: State, to: State) {
    unsafe { (*builder).add_transition(from, to) };
}

type LogicMonitorOnStatusChanged = unsafe extern "C" fn(data: *mut c_void, from: i32, to: i32);
type LogicMonitorOnStateChanged = unsafe extern "C" fn(data: *mut c_void, from: State, to: State);

struct LogicMonitorHooks {
    on_status_changed: LogicMonitorOnStatusChanged,
    on_status_changed_data: *mut c_void,
    on_state_changed: LogicMonitorOnStateChanged,
    on_state_changed_data: *mut c_void,
}

// Safety: Right now it's up to the FFI user to ensure the hooks are safe to call.
unsafe impl Send for LogicMonitorHooks {}
unsafe impl Sync for LogicMonitorHooks {}

impl crate::logic_monitor::Hook for LogicMonitorHooks {
    fn on_status_change(&self, from: Status, to: Status) {
        unsafe { (self.on_status_changed)(self.on_status_changed_data, from as i32, to as i32) };
    }

    fn on_state_change(&self, from: State, to: State) {
        unsafe { (self.on_state_changed)(self.on_state_changed_data, from, to) };
    }
}

#[unsafe(no_mangle)]
extern "C" fn hm_lmb_add_hook(
    builder: *mut LogicMonitorBuilder,
    on_status_changed: LogicMonitorOnStatusChanged,
    on_status_changed_data: *mut c_void,
    on_state_changed: LogicMonitorOnStateChanged,
    on_state_changed_data: *mut c_void,
) {
    unsafe {
        (*builder).add_hook(Box::new(LogicMonitorHooks {
            on_status_changed,
            on_status_changed_data,
            on_state_changed,
            on_state_changed_data,
        }));
    }
}

#[unsafe(no_mangle)]
extern "C" fn hm_lmb_build(builder_ptr: *mut *mut LogicMonitorBuilder) -> *mut LogicMonitor {
    let builder = unsafe { Box::from_raw(*builder_ptr) };
    unsafe {
        *builder_ptr = null_mut();
    }

    // TODO: Propagate error.
    match builder.build() {
        Ok(monitor) => Box::into_raw(Box::new(monitor)),
        Err(_err) => todo!("Report error"),
    }
}

//
// LogicMonitor
//

#[unsafe(no_mangle)]
extern "C" fn hm_lm_delete(monitor: *mut *mut LogicMonitor) {
    let _monitor = unsafe { Box::from_raw(*monitor) };
    unsafe {
        *monitor = null_mut();
    }
}

#[unsafe(no_mangle)]
extern "C" fn hm_lm_transition(monitor: *mut LogicMonitor, to: State) {
    // TODO: Propagate error.
    let _ = unsafe { (*monitor).transition(to) };
}

#[unsafe(no_mangle)]
extern "C" fn hm_lm_enable(monitor: *mut LogicMonitor) {
    // TODO: Propagate error.
    let _ = unsafe { (*monitor).enable() };
}

#[unsafe(no_mangle)]
extern "C" fn hm_lm_disable(monitor: *mut LogicMonitor) {
    // TODO: Propagate error.
    let _ = unsafe { (*monitor).disable() };
}

#[unsafe(no_mangle)]
extern "C" fn hm_lm_status(monitor: *const LogicMonitor) -> i32 {
    unsafe { (*monitor).status() as i32 }
}

#[unsafe(no_mangle)]
extern "C" fn hm_lm_state(monitor: *const LogicMonitor) -> State {
    unsafe { (*monitor).state() }
}

//
// HeartbeatMonitor
//

#[unsafe(no_mangle)]
extern "C" fn hm_hbm_enable(monitor: *mut HeartbeatMonitor) {
    // TODO: Propagate error.
    let _ = unsafe { (*monitor).enable() };
}

#[unsafe(no_mangle)]
extern "C" fn hm_hbm_disable(monitor: *mut HeartbeatMonitor) {
    // TODO: Propagate error.
    let _ = unsafe { (*monitor).disable() };
}

#[unsafe(no_mangle)]
extern "C" fn hm_hbm_send_heartbeat(monitor: *mut HeartbeatMonitor) {
    // TODO: Propagate error.
    let _ = unsafe { (*monitor).send_heartbeat() };
}

#[unsafe(no_mangle)]
extern "C" fn hm_hbm_heartbeat_cycle(monitor: *mut HeartbeatMonitor) -> u64 {
    // TODO: Propagate error.
    unsafe { (*monitor).heartbeat_cycle().as_millis() as u64 }
}

#[unsafe(no_mangle)]
extern "C" fn hm_hbm_last_heartbeat(monitor: *mut HeartbeatMonitor) -> u64 {
    // TODO: Propagate error.
    unsafe { (*monitor).last_heartbeat().elapsed().as_millis() as u64 }
}

#[unsafe(no_mangle)]
extern "C" fn hm_hbm_delete(monitor: *mut *mut HeartbeatMonitor) {
    let _monitor = unsafe { Box::from_raw(*monitor) };
    unsafe {
        *monitor = null_mut();
    }
}

#[unsafe(no_mangle)]
extern "C" fn hm_hbm_new(maximum_heartbeat_cycle_ms: u64) -> *mut HeartbeatMonitor {
    Box::into_raw(Box::new(HeartbeatMonitor::new(Duration::from_millis(
        maximum_heartbeat_cycle_ms,
    ))))
}

//
// HealthMonitor
//

#[unsafe(no_mangle)]
extern "C" fn hm_new(
    deadline_monitor: *const DeadlineMonitor,
    logic_monitor: *const LogicMonitor,
    heartbeat_monitor: *const HeartbeatMonitor,
    alive_monitor: *const AliveMonitorFfi,
    report_interval_ms: u64,
) -> *mut HealthMonitor {
    let (deadline_monitor, logic_monitor, heartbeat_monitor, alive_monitor) = unsafe {
        (
            &*deadline_monitor,
            &*logic_monitor,
            &*heartbeat_monitor,
            &*alive_monitor,
        )
    };

    Box::into_raw(Box::new(HealthMonitor::new(
        deadline_monitor,
        logic_monitor,
        heartbeat_monitor,
        alive_monitor,
        Duration::from_millis(report_interval_ms),
    )))
}

#[unsafe(no_mangle)]
extern "C" fn hm_delete(monitor: *mut *mut HealthMonitor) {
    let _monitor = unsafe { Box::from_raw(*monitor) };
    unsafe {
        *monitor = null_mut();
    }
}

#[unsafe(no_mangle)]
extern "C" fn hm_status(monitor: *const HealthMonitor) -> i32 {
    unsafe { (*monitor).status() as i32 }
}
