#ifndef SCORE_HM_DEADLINE_DEADLINE_MONITOR_H
#define SCORE_HM_DEADLINE_DEADLINE_MONITOR_H

#include <score/hm/common.h>


namespace score::hm::deadline {

using FFIHandler = void*;

// Forward declarations
class DeadlineMonitor;
struct TimeRange;

class DeadlineMonitorBuilder {
public:
    /// Creates a new DeadlineMonitorBuilder
    DeadlineMonitorBuilder();

    /// Adds a deadline with the given tag and duration range to the monitor.
    /// Returns *this for method chaining.
    DeadlineMonitorBuilder& add_deadline(const IdentTag& tag, const TimeRange& range);

    /// Builds the DeadlineMonitor with the configured deadlines.
    DeadlineMonitor build();

private:
    FFIHandler ffi_handler_;
};

} // namespace score::hm::deadline

#endif  // SCORE_HM_DEADLINE_DEADLINE_MONITOR_H