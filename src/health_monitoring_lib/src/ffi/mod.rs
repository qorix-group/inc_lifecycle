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
use crate::common::{DurationRange, Error, Status};
use crate::heartbeat_monitor::HeartbeatMonitor;
use containers::inline::InlineResult;
use core::{ptr::null_mut, time::Duration};

//
// HeartbeatMonitor
//

#[unsafe(no_mangle)]
extern "C" fn hm_hbm_enable(monitor: *mut HeartbeatMonitor) -> Error {
    let result = unsafe { (*monitor).enable() };
    match result {
        Ok(_) => Error::_NoError,
        Err(err) => err,
    }
}

#[unsafe(no_mangle)]
extern "C" fn hm_hbm_disable(monitor: *mut HeartbeatMonitor) -> Error {
    let result = unsafe { (*monitor).disable() };
    match result {
        Ok(_) => Error::_NoError,
        Err(err) => err,
    }
}

#[unsafe(no_mangle)]
extern "C" fn hm_hbm_report_heartbeat(monitor: *mut HeartbeatMonitor) -> InlineResult<Status, Error> {
    unsafe { InlineResult::from_result((*monitor).report_heartbeat()) }
}

#[unsafe(no_mangle)]
extern "C" fn hm_hbm_delete(monitor: *mut *mut HeartbeatMonitor) {
    let _monitor = unsafe { Box::from_raw(*monitor) };
    unsafe {
        *monitor = null_mut();
    }
}

#[unsafe(no_mangle)]
extern "C" fn hm_hbm_new(min_heartbeat_cycle_ms: u64, max_heartbeat_cycle_ms: u64) -> *mut HeartbeatMonitor {
    Box::into_raw(Box::new(HeartbeatMonitor::new(DurationRange::new(
        Duration::from_millis(min_heartbeat_cycle_ms),
        Duration::from_millis(max_heartbeat_cycle_ms),
    ))))
}
