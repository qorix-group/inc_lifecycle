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

use std::{ptr::null_mut, ffi::{CStr, c_char}};
use crate::{common::*, logic_monitor::*};

#[unsafe(no_mangle)]
extern fn hm_lm_state_from_str(name: *const c_char) -> State {
    let name = unsafe { CStr::from_ptr(name) };

    match name.to_str() {
        Ok(name) => State::from_str(name),
        Err(_err) => todo!("Report error"),
    }
}

//
// LogicMonitorBuilder
//

#[unsafe(no_mangle)]
extern fn hm_lmb_new(initial_state: State) -> *mut LogicMonitorBuilder {
    Box::into_raw(Box::new(LogicMonitorBuilder::new(initial_state)))
}

#[unsafe(no_mangle)]
extern fn hm_lmb_delete(builder_ptr: *mut *mut LogicMonitorBuilder) {
    let _builder = unsafe { Box::from_raw(*builder_ptr) };
    unsafe { *builder_ptr = null_mut(); }
}

#[unsafe(no_mangle)]
extern fn hm_lmb_add_transition(builder: *mut LogicMonitorBuilder, from: State, to: State) {
    unsafe { (*builder).add_transition(from, to) };
}

#[unsafe(no_mangle)]
extern fn hm_lmb_build(builder_ptr: *mut *mut LogicMonitorBuilder, out: *mut *mut LogicMonitor) -> Error {
    let builder = unsafe { Box::from_raw(*builder_ptr) };
    unsafe { *builder_ptr = null_mut(); }

    match builder.build() {
        Ok(monitor) => {
            unsafe { *out = Box::into_raw(Box::new(monitor)) }
            Error::_NoError
        },
        Err(err) => err
    }
}

//
// LogicMonitor
//

#[unsafe(no_mangle)]
extern fn hm_lm_delete(monitor: *mut *mut LogicMonitor) {
    let _monitor = unsafe { Box::from_raw(*monitor) };
    unsafe { *monitor = null_mut(); }
}

#[unsafe(no_mangle)]
extern fn hm_lm_transition(monitor: *mut LogicMonitor, to: State) -> Error {
    match unsafe { (*monitor).transition(to) } {
        Ok(_) => Error::_NoError,
        Err(err) => err
    }
}

#[unsafe(no_mangle)]
extern fn hm_lm_enable(monitor: *mut LogicMonitor) -> Error {
    match unsafe { (*monitor).enable() } {
        Ok(_) => Error::_NoError,
        Err(err) => err
    }
}

#[unsafe(no_mangle)]
extern fn hm_lm_disable(monitor: *mut LogicMonitor) -> Error {
    match unsafe { (*monitor).disable() } {
        Ok(_) => Error::_NoError,
        Err(err) => err
    }
}

#[unsafe(no_mangle)]
extern fn hm_lm_status(monitor: *const LogicMonitor) -> i32 {
    unsafe { (*monitor).status() as i32 }
}

#[unsafe(no_mangle)]
extern fn hm_lm_state(monitor: *const LogicMonitor) -> State {
    unsafe { (*monitor).state() }
}
