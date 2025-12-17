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
use crate::common;
use crate::logic_monitor::*;
use crate::{deadline_monitor::*, heartbeat_monitor::*};
use alive_monitor::alive_monitor::AliveMonitor;
use std::{
    sync::{
        Arc,
        atomic::{AtomicU32, Ordering},
    },
    thread::{self},
    time::{Duration, Instant},
};

pub struct HealthMonitor {
    status: Arc<AtomicU32>,
}

impl HealthMonitor {
    // TODO: Maybe borrow and clone internally?
    pub fn new(
        deadline_monitor: &DeadlineMonitor,
        logic_monitor: &LogicMonitor,
        heartbeat_monitor: &HeartbeatMonitor,
        alive_monitor: &AliveMonitor,
        report_interval: Duration,
    ) -> Self {
        let status = Arc::new(AtomicU32::new(common::Status::Running.into()));
        let deadline_monitor_clone = deadline_monitor.clone();
        let logic_monitor_clone = logic_monitor.clone();
        let mut heartbeat_monitor_clone = heartbeat_monitor.clone();
        let alive_monitor_clone = alive_monitor.clone();
        let status_clone = Arc::clone(&status);

        let heartbeat_monitor_cycle = heartbeat_monitor_clone.heartbeat_cycle();
        let cycle_time = if heartbeat_monitor_cycle > report_interval {
            report_interval
        } else {
            heartbeat_monitor_cycle
        };
        let _ = thread::spawn(move || {
            loop {
                let start_time = Instant::now();

                if deadline_monitor_clone.step() == common::Status::Failed {
                    status_clone.store(common::Status::Failed.into(), Ordering::Release);
                }

                if logic_monitor_clone.status() == common::Status::Failed {
                    status_clone.store(common::Status::Failed.into(), Ordering::Release);
                }

                if heartbeat_monitor_clone.step() == common::Status::Failed {
                    status_clone.store(common::Status::Failed.into(), Ordering::Release);
                }

                // TODO: Report failure. Right now just not reporting alive status.
                if status_clone.load(Ordering::Acquire) == common::Status::Running.into() {
                    alive_monitor_clone.keep_alive();

                    let sleep_duration =
                        cycle_time.saturating_sub(Instant::now().duration_since(start_time));
                    if sleep_duration > Duration::ZERO {
                        thread::sleep(sleep_duration);
                    }
                } else {
                    return;
                }
            }
        });

        Self { status }
    }

    pub fn status(&self) -> common::Status {
        common::Status::from(self.status.load(Ordering::Acquire))
    }
}

impl Drop for HealthMonitor {
    fn drop(&mut self) {
        self.status
            .store(common::Status::Stopped.into(), Ordering::Release);
        // TODO: join?
    }
}

#[cfg(test)]
mod tests {}
