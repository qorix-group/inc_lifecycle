use std::sync::Arc;
use std::sync::atomic::{AtomicBool, AtomicU64, Ordering};
use std::time::{Duration, Instant};

#[derive(PartialEq, Clone, Copy)]
pub enum HeartbeatMonitorStatus {
    Healthy = 0,
    TimedOut,
    Disabled,
    Enabled,
}

struct AtomicInstant {
    nanos: AtomicU64,
    epoch: Instant,
}

impl AtomicInstant {
    fn new() -> Self {
        Self {
            nanos: AtomicU64::new(0),
            epoch: Instant::now(),
        }
    }

    fn store(&self, instant: Instant) {
        let nanos = instant.duration_since(self.epoch).as_nanos() as u64;
        self.nanos.store(nanos, Ordering::Release);
    }

    fn load(&self) -> Instant {
        return self.epoch + Duration::from_nanos(self.nanos.load(Ordering::Acquire));
    }
}

#[derive(Clone)]
pub struct HeartbeatMonitor {
    maximum_heartbeat_cycle_ms: Duration,
    last_heartbeat: Arc<AtomicInstant>,
    is_enabled: Arc<AtomicBool>,
}

impl HeartbeatMonitor {
    pub fn new(maximum_heartbeat_cycle_ms: Duration) -> Self {
        Self {
            maximum_heartbeat_cycle_ms,
            last_heartbeat: Arc::new(AtomicInstant::new()),
            is_enabled: Arc::new(AtomicBool::new(true)),
        }
    }

    pub fn get_heartbeat_cycle(&self) -> Duration {
        self.maximum_heartbeat_cycle_ms
    }

    pub fn get_last_heartbeat(&self) -> Instant {
        self.last_heartbeat.load()
    }

    pub fn enable(&mut self) {
        self.is_enabled.store(true, Ordering::Release);
        println!("Heartbeat monitor enabled.");
    }

    pub fn disable(&mut self) {
        self.is_enabled.store(false, Ordering::Release);
        println!("Heartbeat monitor disabled.");
    }

    pub fn check_heartbeat(&mut self) -> HeartbeatMonitorStatus {
        if !self.is_enabled.load(Ordering::Acquire) {
            return HeartbeatMonitorStatus::Disabled;
        }
        let now = Instant::now();
        if now - self.get_last_heartbeat() > self.maximum_heartbeat_cycle_ms {
            HeartbeatMonitorStatus::TimedOut
        } else {
            HeartbeatMonitorStatus::Healthy
        }
    }

    pub fn heartbeat(&mut self) {
        if self.is_enabled.load(Ordering::Acquire) {
            return;
        }
        self.last_heartbeat.store(Instant::now());
    }
}
