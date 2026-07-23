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
#include "testing_factory.h"
#include <memory>
#include <utility>
#include <vector>

namespace score::mw::health::testing_support
{

/// Owns the MockDeadlineMonitor/MockHeartbeatMonitor/MockLogicMonitor instances handed out for
/// each MonitorTag, and can produce a real, production `HealthMonitor` bound to itself via
/// `as_health_monitor()`.
class MockHealthMonitor
{
  public:
    /// Get-or-create the MockDeadlineMonitor registered for `monitor_tag`, to set expectations on
    /// (directly, or on its own nested MockDeadline instances via `mock_deadline_for`).
    deadline::testing_support::MockDeadlineMonitor& mock_deadline_monitor_for(const MonitorTag& monitor_tag)
    {
        return get_or_create(deadline_monitors_, monitor_tag);
    }

    /// Get-or-create the MockHeartbeatMonitor registered for `monitor_tag`, to set expectations on.
    heartbeat::testing_support::MockHeartbeatMonitor& mock_heartbeat_monitor_for(const MonitorTag& monitor_tag)
    {
        return get_or_create(heartbeat_monitors_, monitor_tag);
    }

    /// Get-or-create the MockLogicMonitor registered for `monitor_tag`, to set expectations on.
    logic::testing_support::MockLogicMonitor& mock_logic_monitor_for(const MonitorTag& monitor_tag)
    {
        return get_or_create(logic_monitors_, monitor_tag);
    }

    /// Used by mock_health_monitor_ffi.cpp to route `health_monitor_get_deadline_monitor`.
    deadline::testing_support::MockDeadlineMonitor* find_deadline_monitor(const MonitorTag& monitor_tag)
    {
        return find(deadline_monitors_, monitor_tag);
    }

    /// Used by mock_health_monitor_ffi.cpp to route `health_monitor_get_heartbeat_monitor`.
    heartbeat::testing_support::MockHeartbeatMonitor* find_heartbeat_monitor(const MonitorTag& monitor_tag)
    {
        return find(heartbeat_monitors_, monitor_tag);
    }

    /// Used by mock_health_monitor_ffi.cpp to route `health_monitor_get_logic_monitor`.
    logic::testing_support::MockLogicMonitor* find_logic_monitor(const MonitorTag& monitor_tag)
    {
        return find(logic_monitors_, monitor_tag);
    }

    /// Real production `DeadlineMonitor`, pinned to the (get-or-create) mock for `monitor_tag`.
    deadline::DeadlineMonitor get_deadline_monitor(const MonitorTag& monitor_tag)
    {
        mock_deadline_monitor_for(monitor_tag);
        auto health_monitor{as_health_monitor()};
        auto result{health_monitor.get_deadline_monitor(monitor_tag)};
        return std::move(result.value());
    }

    /// Real production `HeartbeatMonitor`, pinned to the (get-or-create) mock for `monitor_tag`.
    heartbeat::HeartbeatMonitor get_heartbeat_monitor(const MonitorTag& monitor_tag)
    {
        mock_heartbeat_monitor_for(monitor_tag);
        auto health_monitor{as_health_monitor()};
        auto result{health_monitor.get_heartbeat_monitor(monitor_tag)};
        return std::move(result.value());
    }

    /// Real production `LogicMonitor`, pinned to the (get-or-create) mock for `monitor_tag`.
    logic::LogicMonitor get_logic_monitor(const MonitorTag& monitor_tag)
    {
        mock_logic_monitor_for(monitor_tag);
        auto health_monitor{as_health_monitor()};
        auto result{health_monitor.get_logic_monitor(monitor_tag)};
        return std::move(result.value());
    }

    /// Real production `HealthMonitor` whose FFIHandle is this mock's own address.
    HealthMonitor as_health_monitor()
    {
        return internal::ConstructibleFrom<HealthMonitor>::create(reinterpret_cast<internal::FFIHandle>(this));
    }

  private:
    template <typename Mock>
    using Registry = std::vector<std::pair<MonitorTag, std::unique_ptr<Mock>>>;

    template <typename Mock>
    static Mock& get_or_create(Registry<Mock>& registry, const MonitorTag& monitor_tag)
    {
        if (auto* existing{find(registry, monitor_tag)}; existing != nullptr)
        {
            return *existing;
        }
        registry.emplace_back(monitor_tag, std::make_unique<Mock>());
        return *registry.back().second;
    }

    template <typename Mock>
    static Mock* find(Registry<Mock>& registry, const MonitorTag& monitor_tag)
    {
        for (auto& [tag, mock] : registry)
        {
            if (tag == monitor_tag)
            {
                return mock.get();
            }
        }
        return nullptr;
    }

    Registry<deadline::testing_support::MockDeadlineMonitor> deadline_monitors_;
    Registry<heartbeat::testing_support::MockHeartbeatMonitor> heartbeat_monitors_;
    Registry<logic::testing_support::MockLogicMonitor> logic_monitors_;
};

/// Produces a real, production `HealthMonitorBuilder` pinned to a test-held `MockHealthMonitor`.
/// The builder routes `add_*_monitor`/`build` through the mock FFI to that same mock (see
/// mock_health_monitor_ffi.cpp), so monitors registered while building remain reachable for
/// setting expectations via the underlying `MockHealthMonitor`.
class MockHealthMonitorBuilder
{
  public:
    explicit MockHealthMonitorBuilder(MockHealthMonitor& mock) : mock_(mock)
    {
    }

    HealthMonitorBuilder as_health_monitor_builder()
    {
        return internal::ConstructibleFrom<HealthMonitorBuilder>::create(reinterpret_cast<internal::FFIHandle>(&mock_));
    }

  private:
    MockHealthMonitor& mock_;
};

}  // namespace score::mw::health::testing_support

#endif  // SCORE_HM_TESTS_MOCKS_MOCK_HEALTH_MONITOR_H
