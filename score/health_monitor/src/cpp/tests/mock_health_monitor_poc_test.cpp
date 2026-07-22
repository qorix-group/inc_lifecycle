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
#include "score/health_monitor/src/cpp/tests/mocks/mock_health_monitor.h"
#include <gtest/gtest.h>

using namespace score::mw::health;
using score::mw::health::deadline::testing_support::MockDeadline;
using score::mw::health::testing_support::MockHealthMonitor;
using ::testing::Return;

// This test links health_monitor.cpp/deadline_monitor.cpp against our mock FFI shims instead of
// the real Rust FFI library. Test code only ever touches the real, unmodified
// `HealthMonitorBuilder`/`HealthMonitor`/`DeadlineMonitor`/`Deadline` classes -- exactly as
// application code would. The mock is reached purely through the FFI boundary, never by changing
// any of those classes' public API or internals.
TEST(MockHealthMonitorPoc, DeadlineChain_RoutesThroughTagsToLeafMock)
{
    MockHealthMonitor mock_health_monitor;
    MockDeadline& deadline_mock{mock_health_monitor.MockDeadlineMonitorFor(MonitorTag("deadline_monitor"))
                                    .MockDeadlineFor(DeadlineTag("operation"))};

    {
        ::testing::InSequence in_order;
        EXPECT_CALL(deadline_mock, Start()).WillOnce(Return(internal::kSuccess));
        EXPECT_CALL(deadline_mock, Stop()).WillOnce(Return(internal::kSuccess));
        EXPECT_CALL(deadline_mock, Destroy()).WillOnce(Return(internal::kSuccess));
    }

    HealthMonitor health_monitor{mock_health_monitor.AsHealthMonitor()};

    auto deadline_monitor_result{health_monitor.get_deadline_monitor(MonitorTag("deadline_monitor"))};
    ASSERT_TRUE(deadline_monitor_result.has_value());
    deadline::DeadlineMonitor deadline_monitor{std::move(deadline_monitor_result.value())};

    auto deadline_result{deadline_monitor.get_deadline(DeadlineTag("operation"))};
    ASSERT_TRUE(deadline_result.has_value());
    deadline::Deadline deadline{std::move(deadline_result.value())};

    auto handle_result{deadline.start()};
    ASSERT_TRUE(handle_result.has_value());
    handle_result->stop();
}  // ~Deadline() runs at end of scope, routing deadline_destroy() to MockDeadline::Destroy().

TEST(MockHealthMonitorPoc, GetDeadlineMonitorAndGetDeadline_ReturnRealObjectsPinnedToMocks)
{
    MockHealthMonitor mock_health_monitor;

    // Set expectations on the leaf mock before ever touching the real API surface.
    MockDeadline& deadline_mock{mock_health_monitor.MockDeadlineMonitorFor(MonitorTag("deadline_monitor"))
                                    .MockDeadlineFor(DeadlineTag("operation"))};
    {
        ::testing::InSequence in_order;
        EXPECT_CALL(deadline_mock, Start()).WillOnce(Return(internal::kSuccess));
        EXPECT_CALL(deadline_mock, Stop()).WillOnce(Return(internal::kSuccess));
        EXPECT_CALL(deadline_mock, Destroy()).WillOnce(Return(internal::kSuccess));
    }

    // GetDeadlineMonitor() hands back a real, production DeadlineMonitor pinned to that mock.
    deadline::DeadlineMonitor deadline_monitor{mock_health_monitor.GetDeadlineMonitor(MonitorTag("deadline_monitor"))};

    auto deadline_result{deadline_monitor.get_deadline(DeadlineTag("operation"))};
    ASSERT_TRUE(deadline_result.has_value());
    deadline::Deadline deadline{std::move(deadline_result.value())};

    auto handle_result{deadline.start()};
    ASSERT_TRUE(handle_result.has_value());
    handle_result->stop();
}

TEST(MockHealthMonitorPoc, UnregisteredMonitorTag_ReturnsNotFound)
{
    MockHealthMonitor mock_health_monitor;
    HealthMonitor health_monitor{mock_health_monitor.AsHealthMonitor()};

    auto result{health_monitor.get_deadline_monitor(MonitorTag("unregistered"))};
    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(result.error(), Error::NotFound);
}

TEST(MockHealthMonitorPoc, UnregisteredDeadlineTag_ReturnsNotFound)
{
    MockHealthMonitor mock_health_monitor;
    mock_health_monitor.MockDeadlineMonitorFor(MonitorTag("deadline_monitor"));

    HealthMonitor health_monitor{mock_health_monitor.AsHealthMonitor()};
    auto deadline_monitor_result{health_monitor.get_deadline_monitor(MonitorTag("deadline_monitor"))};
    ASSERT_TRUE(deadline_monitor_result.has_value());
    deadline::DeadlineMonitor deadline_monitor{std::move(deadline_monitor_result.value())};

    auto deadline_result{deadline_monitor.get_deadline(DeadlineTag("unregistered"))};
    ASSERT_FALSE(deadline_result.has_value());
    ASSERT_EQ(deadline_result.error(), Error::NotFound);
}
