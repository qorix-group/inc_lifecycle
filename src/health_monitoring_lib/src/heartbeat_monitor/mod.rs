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
use crate::common::*;
use core::sync::atomic::{AtomicBool, AtomicU64, Ordering};
use core::time::Duration;
use std::sync::Arc;
use std::time::Instant;

struct AtomicInstant {
    nanos: AtomicU64,
    epoch: Instant,
}

impl AtomicInstant {
    fn new() -> Self {
        Self {
            nanos: AtomicU64::new(u64::MAX),
            epoch: Instant::now(),
        }
    }

    fn store(&self, instant: Instant) {
        let dur = instant.checked_duration_since(self.epoch).unwrap_or(Duration::ZERO);
        self.nanos.store(dur.as_nanos() as u64, Ordering::Release);
    }

    #[allow(dead_code)]
    fn load(&self) -> Option<Instant> {
        let val = self.nanos.load(Ordering::Acquire);
        if val == u64::MAX {
            None
        } else {
            Some(self.epoch + Duration::from_nanos(val))
        }
    }
}

#[derive(Clone)]
pub struct HeartbeatMonitor {
    #[allow(dead_code)]
    duration_range: DurationRange,
    last_heartbeat: Arc<AtomicInstant>,
    is_enabled: Arc<AtomicBool>,
    #[allow(dead_code)]
    created_at: Instant,
}

impl HeartbeatMonitor {
    pub fn new(duration_range: DurationRange) -> Self {
        Self {
            duration_range,
            last_heartbeat: Arc::new(AtomicInstant::new()),
            is_enabled: Arc::new(AtomicBool::new(true)),
            created_at: Instant::now(),
        }
    }

    #[allow(dead_code)]
    fn last_heartbeat(&self) -> Option<Instant> {
        self.last_heartbeat.load()
    }

    #[allow(dead_code)]
    pub(crate) fn heartbeat_cycle_max(&self) -> Duration {
        self.duration_range.max
    }

    #[allow(dead_code)]
    fn heartbeat_cycle_min(&self) -> Duration {
        self.duration_range.min
    }

    pub fn enable(&self) -> Result<(), Error> {
        self.is_enabled.store(true, Ordering::Release);
        println!("Heartbeat monitor enabled.");
        Ok(())
    }

    pub fn disable(&self) -> Result<(), Error> {
        self.is_enabled.store(false, Ordering::Release);
        println!("Heartbeat monitor disabled.");
        Ok(())
    }

    #[allow(dead_code)]
    pub(crate) fn evaluate(&self) -> Status {
        if !self.is_enabled.load(Ordering::Acquire) {
            return Status::Disabled;
        }
        let now = Instant::now();
        match self.last_heartbeat() {
            None => {
                if now.duration_since(self.created_at) > self.duration_range.max {
                    return Status::Failed;
                }
                Status::Running
            },
            Some(time) => {
                let elapsed = now.saturating_duration_since(time);
                if elapsed > self.heartbeat_cycle_max() || elapsed < self.heartbeat_cycle_min() {
                    Status::Failed
                } else {
                    Status::Running
                }
            },
        }
    }

    pub fn report_heartbeat(&self) -> Result<Status, Error> {
        if !self.is_enabled.load(Ordering::Acquire) {
            return Ok(Status::Disabled);
        }
        self.last_heartbeat.store(Instant::now());
        Ok(Status::Running)
    }
}

#[cfg(test)]
mod tests;
