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
#include "score/mw/health/health_monitor.h"
#include "score/mw/health/deadline_monitor.h"
#include "score/mw/health/logic_monitor.h"
#include "score/mw/health/mock/mock_health_monitor.h"

#include <gtest/gtest.h>
#include <utility>

using namespace score::mw::health;
using score::mw::health::deadline::testing_support::MockDeadline;
using score::mw::health::logic::testing_support::MockLogicMonitor;
using score::mw::health::testing_support::MockHealthMonitor;
using score::mw::health::testing_support::MockHealthMonitorBuilder;
using ::testing::_;
using ::testing::Return;

/// Unit tests for the top-level aggregator mock (`MockHealthMonitor` / `MockHealthMonitorBuilder`):
/// tag-based routing to the per-monitor mocks, NotFound handling, and the builder-backed flow.
class MockHealthMonitorFixture : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        RecordProperty("TestType", "unit-test");
        RecordProperty("DerivationTechnique", "explorative-testing");
    }
};

TEST_F(MockHealthMonitorFixture, GetDeadlineMonitor_ReturnsRealObjectPinnedToMock)
{
    RecordProperty(
        "Description",
        "Verify MockHealthMonitor::get_deadline_monitor hands back a real DeadlineMonitor pinned to the mock.");

    MockHealthMonitor mock;
    MockDeadline& deadline_mock{
        mock.mock_deadline_monitor_for(MonitorTag("deadline_monitor")).mock_deadline_for(DeadlineTag("operation"))};
    {
        ::testing::InSequence in_order;
        EXPECT_CALL(deadline_mock, start()).WillOnce(Return(internal::kSuccess));
        EXPECT_CALL(deadline_mock, stop()).WillOnce(Return(internal::kSuccess));
        EXPECT_CALL(deadline_mock, destroy()).WillOnce(Return(internal::kSuccess));
    }

    deadline::DeadlineMonitor deadline_monitor{mock.get_deadline_monitor(MonitorTag("deadline_monitor"))};

    auto deadline_result{deadline_monitor.get_deadline(DeadlineTag("operation"))};
    ASSERT_TRUE(deadline_result.has_value());
    deadline::Deadline deadline{std::move(deadline_result.value())};

    auto handle_result{deadline.start()};
    ASSERT_TRUE(handle_result.has_value());
    handle_result->stop();
}

TEST_F(MockHealthMonitorFixture, GetLogicMonitor_RoutesToMock)
{
    RecordProperty(
        "Description",
        "Verify get_logic_monitor returns a real LogicMonitor pinned to the mock registered for its tag.");

    MockHealthMonitor mock;
    MockLogicMonitor& logic_mock{mock.mock_logic_monitor_for(MonitorTag("logic"))};
    HealthMonitor health_monitor{mock.as_health_monitor()};

    EXPECT_CALL(logic_mock, transition(_)).WillOnce(Return(score::cpp::expected<StateTag, Error>(StateTag{"stopped"})));

    auto logic_monitor_result{health_monitor.get_logic_monitor(MonitorTag("logic"))};
    ASSERT_TRUE(logic_monitor_result.has_value());
    logic::LogicMonitor logic_monitor{std::move(logic_monitor_result.value())};

    auto transition_result{logic_monitor.transition(StateTag("stopped"))};
    ASSERT_TRUE(transition_result.has_value());
    ASSERT_EQ(transition_result.value(), StateTag("stopped"));
}

TEST_F(MockHealthMonitorFixture, UnregisteredMonitorTag_ReturnsNotFound)
{
    RecordProperty("Description", "Verify get_deadline_monitor returns NotFound for an unregistered monitor tag.");

    MockHealthMonitor mock;
    HealthMonitor health_monitor{mock.as_health_monitor()};

    auto result{health_monitor.get_deadline_monitor(MonitorTag("unregistered"))};
    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(result.error(), Error::NotFound);
}

TEST_F(MockHealthMonitorFixture, BuilderBackedByMock_ProducesMockedHealthMonitor)
{
    RecordProperty(
        "Description",
        "Verify a HealthMonitor built through the real HealthMonitorBuilder can be pinned to a test-held mock.");

    MockHealthMonitor mock;
    MockDeadline& deadline_mock{
        mock.mock_deadline_monitor_for(MonitorTag("deadline")).mock_deadline_for(DeadlineTag("operation"))};
    {
        ::testing::InSequence in_order;
        EXPECT_CALL(deadline_mock, start()).WillOnce(Return(internal::kSuccess));
        EXPECT_CALL(deadline_mock, stop()).WillOnce(Return(internal::kSuccess));
        EXPECT_CALL(deadline_mock, destroy()).WillOnce(Return(internal::kSuccess));
    }

    // Drive the real builder API; it is pinned to `mock` and routes add/build through the mock FFI.
    HealthMonitor health_monitor{MockHealthMonitorBuilder{mock}
                                     .as_health_monitor_builder()
                                     .add_deadline_monitor(MonitorTag("deadline"), deadline::DeadlineMonitorBuilder{})
                                     .build()
                                     .value()};

    auto deadline_monitor_result{health_monitor.get_deadline_monitor(MonitorTag("deadline"))};
    ASSERT_TRUE(deadline_monitor_result.has_value());
    deadline::DeadlineMonitor deadline_monitor{std::move(deadline_monitor_result.value())};

    auto deadline_result{deadline_monitor.get_deadline(DeadlineTag("operation"))};
    ASSERT_TRUE(deadline_result.has_value());
    deadline::Deadline deadline{std::move(deadline_result.value())};

    auto handle_result{deadline.start()};
    ASSERT_TRUE(handle_result.has_value());
    handle_result->stop();
}
