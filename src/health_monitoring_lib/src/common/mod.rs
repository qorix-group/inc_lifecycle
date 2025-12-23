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
use core::time::Duration;

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
#[repr(C)]
pub enum Status {
    Running = 0,
    Disabled,
    Failed,
    Stopped,
}

impl From<Status> for u32 {
    fn from(val: Status) -> Self {
        val as u32
    }
}

impl From<u32> for Status {
    fn from(value: u32) -> Self {
        match value {
            0 => Self::Running,
            1 => Self::Disabled,
            2 => Self::Failed,
            3 => Self::Stopped,
            _ => panic!("Trying to create Status from unknown value {}", value),
        }
    }
}

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
#[repr(C)]
pub enum Error {
    _NoError = 0, // Only used by the FFI API.
    BadParameter,
    NotAllowed,
    OutOfMemory,
    Generic,
}

#[derive(Clone)]
pub struct DurationRange {
    pub min: Duration,
    pub max: Duration,
}

impl DurationRange {
    pub fn new(min: Duration, max: Duration) -> Self {
        Self { min, max }
    }
}
