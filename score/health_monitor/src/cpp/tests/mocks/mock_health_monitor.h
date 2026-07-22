/********************************************************************************
 * Copyright (c) 2026 Contributors to the Eclipse Foundation
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
#ifndef SCORE_HM_TESTS_MOCKS_MOCK_HEALTH_MONITOR_H
#define SCORE_HM_TESTS_MOCKS_MOCK_HEALTH_MONITOR_H

#include "mock_deadline_monitor.h"
#include "mock_heartbeat_monitor.h"
#include "mock_logic_monitor.h"
#include "score/mw/health/health_monitor.h"
#include <map>
#include <memory>

namespace score::mw::health::testing_support
{

/// Owns the MockDeadlineMonitor/MockHeartbeatMonitor/MockLogicMonitor instances handed out for
/// each MonitorTag, and can produce a real, production `HealthMonitor` bound to itself via
/// `AsHealthMonitor()`.
class MockHealthMonitor
{
  public:
    /// Get-or-create the MockDeadlineMonitor registered for `monitor_tag`, to set expectations on
    /// (directly, or on its own nested MockDeadline instances via `MockDeadlineFor`).
    deadline::testing_support::MockDeadlineMonitor& MockDeadlineMonitorFor(const MonitorTag& monitor_tag)
    {
        return *deadline_monitors_.emplace(monitor_tag, std::make_unique<deadline::testing_support::MockDeadlineMonitor>())
                    .first->second;
    }

    /// Get-or-create the MockHeartbeatMonitor registered for `monitor_tag`, to set expectations on.
    heartbeat::testing_support::MockHeartbeatMonitor& MockHeartbeatMonitorFor(const MonitorTag& monitor_tag)
    {
        return *heartbeat_monitors_.emplace(monitor_tag, std::make_unique<heartbeat::testing_support::MockHeartbeatMonitor>())
                    .first->second;
    }

    /// Get-or-create the MockLogicMonitor registered for `monitor_tag`, to set expectations on.
    logic::testing_support::MockLogicMonitor& MockLogicMonitorFor(const MonitorTag& monitor_tag)
    {
        return *logic_monitors_.emplace(monitor_tag, std::make_unique<logic::testing_support::MockLogicMonitor>()).first->second;
    }

    /// Used by mock_health_monitor_ffi.cpp to route `health_monitor_get_deadline_monitor`.
    deadline::testing_support::MockDeadlineMonitor* FindDeadlineMonitor(const MonitorTag& monitor_tag)
    {
        auto it{deadline_monitors_.find(monitor_tag)};
        return it == deadline_monitors_.end() ? nullptr : it->second.get();
    }

    /// Used by mock_health_monitor_ffi.cpp to route `health_monitor_get_heartbeat_monitor`.
    heartbeat::testing_support::MockHeartbeatMonitor* FindHeartbeatMonitor(const MonitorTag& monitor_tag)
    {
        auto it{heartbeat_monitors_.find(monitor_tag)};
        return it == heartbeat_monitors_.end() ? nullptr : it->second.get();
    }

    /// Used by mock_health_monitor_ffi.cpp to route `health_monitor_get_logic_monitor`.
    logic::testing_support::MockLogicMonitor* FindLogicMonitor(const MonitorTag& monitor_tag)
    {
        auto it{logic_monitors_.find(monitor_tag)};
        return it == logic_monitors_.end() ? nullptr : it->second.get();
    }

    /// Real production `DeadlineMonitor`, pinned to the (get-or-create) mock for `monitor_tag`.
    deadline::DeadlineMonitor GetDeadlineMonitor(const MonitorTag& monitor_tag)
    {
        MockDeadlineMonitorFor(monitor_tag);
        auto health_monitor{AsHealthMonitor()};
        auto result{health_monitor.get_deadline_monitor(monitor_tag)};
        return std::move(result.value());
    }

    /// Real production `HeartbeatMonitor`, pinned to the (get-or-create) mock for `monitor_tag`.
    heartbeat::HeartbeatMonitor GetHeartbeatMonitor(const MonitorTag& monitor_tag)
    {
        MockHeartbeatMonitorFor(monitor_tag);
        auto health_monitor{AsHealthMonitor()};
        auto result{health_monitor.get_heartbeat_monitor(monitor_tag)};
        return std::move(result.value());
    }

    /// Real production `LogicMonitor`, pinned to the (get-or-create) mock for `monitor_tag`.
    logic::LogicMonitor GetLogicMonitor(const MonitorTag& monitor_tag)
    {
        MockLogicMonitorFor(monitor_tag);
        auto health_monitor{AsHealthMonitor()};
        auto result{health_monitor.get_logic_monitor(monitor_tag)};
        return std::move(result.value());
    }

    /// Real production `HealthMonitor` whose FFIHandle is this mock's own address.
    HealthMonitor AsHealthMonitor()
    {
        return internal::ConstructibleFrom<HealthMonitor>::create(reinterpret_cast<internal::FFIHandle>(this));
    }

  private:
    std::map<MonitorTag, std::unique_ptr<deadline::testing_support::MockDeadlineMonitor>> deadline_monitors_;
    std::map<MonitorTag, std::unique_ptr<heartbeat::testing_support::MockHeartbeatMonitor>> heartbeat_monitors_;
    std::map<MonitorTag, std::unique_ptr<logic::testing_support::MockLogicMonitor>> logic_monitors_;
};

}  // namespace score::mw::health::testing_support

#endif  // SCORE_HM_TESTS_MOCKS_MOCK_HEALTH_MONITOR_H
