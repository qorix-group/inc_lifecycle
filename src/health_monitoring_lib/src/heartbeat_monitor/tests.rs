#[cfg(test)]
mod test {
    use crate::common::*;
    use crate::heartbeat_monitor::HeartbeatMonitor;
    use core::time::Duration;
    use std::sync::Arc;
    use std::thread;

    #[test]
    fn test_heartbeat_success() {
        let range = DurationRange {
            min: Duration::from_millis(10),
            max: Duration::from_millis(100),
        };
        let monitor = HeartbeatMonitor::new(range);
        thread::sleep(Duration::from_millis(20));

        let status = monitor.report_heartbeat().unwrap();
        assert_eq!(status, Status::Running);

        thread::sleep(Duration::from_millis(50));
        assert_eq!(monitor.evaluate(), Status::Running);
    }

    #[test]
    fn heartbeat_failed_max() {
        let range = DurationRange {
            min: Duration::ZERO,
            max: Duration::from_millis(50),
        };
        let monitor = HeartbeatMonitor::new(range);

        monitor.report_heartbeat().unwrap();
        thread::sleep(Duration::from_millis(70));
        assert_eq!(
            monitor.evaluate(),
            Status::Failed,
            "Monitor should fail if heartbeat is stale"
        );
    }

    #[test]
    fn test_heartbeat_failed_min() {
        let range = DurationRange {
            min: Duration::from_millis(100),
            max: Duration::from_millis(500),
        };
        let monitor = HeartbeatMonitor::new(range);

        monitor.report_heartbeat().unwrap();
        assert_eq!(monitor.evaluate(), Status::Failed, "Should fail if checked too soon");
    }

    #[test]
    fn test_heartbeat_disabled_failed() {
        let range = DurationRange {
            min: Duration::ZERO,
            max: Duration::from_millis(10),
        };
        let monitor = HeartbeatMonitor::new(range);

        monitor.disable().unwrap();
        thread::sleep(Duration::from_millis(20));

        assert_eq!(monitor.evaluate(), Status::Disabled);
        assert_eq!(monitor.report_heartbeat().unwrap(), Status::Disabled);
    }

    #[test]
    fn test_concurrency() {
        let range = DurationRange {
            min: Duration::ZERO,
            max: Duration::from_millis(20),
        };

        let monitor = HeartbeatMonitor::new(range);
        let monitor_shared = Arc::new(monitor);
        let app_monitor = Arc::clone(&monitor_shared);
        let health_monitor = Arc::clone(&monitor_shared);

        let writer_handle = thread::spawn(move || {
            for _ in 0..10 {
                let _ = app_monitor.report_heartbeat();
                thread::sleep(Duration::from_millis(10));
            }
            app_monitor.disable().unwrap();
        });

        let reader_handle: thread::JoinHandle<()> = thread::spawn(move || {
            for _ in 0..20 {
                let status = health_monitor.evaluate();
                assert!(matches!(status, Status::Running | Status::Disabled));
                thread::sleep(Duration::from_millis(20));
            }
        });

        writer_handle.join().unwrap();
        reader_handle.join().unwrap();

        assert_eq!(monitor_shared.evaluate(), Status::Disabled);
    }
}
