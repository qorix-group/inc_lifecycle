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
#include "score/mw/health/deadline_monitor.h"
#include "score/mw/health/mock/mock_deadline_monitor.h"

#include <gtest/gtest.h>
#include <utility>

using namespace score::mw::health;
using score::mw::health::deadline::testing_support::MockDeadline;
using score::mw::health::deadline::testing_support::MockDeadlineMonitor;
using ::testing::Return;

/// Unit tests for the deadline mock double (`MockDeadlineMonitor` / `MockDeadline`), driven in
/// isolation through the real `DeadlineMonitor` -- without a `MockHealthMonitor`.
class MockDeadlineMonitorFixture : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        RecordProperty("TestType", "unit-test");
        RecordProperty("DerivationTechnique", "explorative-testing");
    }
};

TEST_F(MockDeadlineMonitorFixture, GetDeadline_ReturnsRealObjectPinnedToMock)
{
    RecordProperty(
        "Description",
        "Verify MockDeadlineMonitor hands back a real Deadline pinned to its mock, forwarding start/stop/destroy.");

    MockDeadlineMonitor mock_monitor;
    MockDeadline& deadline_mock{mock_monitor.mock_deadline_for(DeadlineTag("operation"))};
    {
        ::testing::InSequence in_order;
        EXPECT_CALL(deadline_mock, start()).WillOnce(Return(internal::kSuccess));
        EXPECT_CALL(deadline_mock, stop()).WillOnce(Return(internal::kSuccess));
        EXPECT_CALL(deadline_mock, destroy()).WillOnce(Return(internal::kSuccess));
    }

    deadline::DeadlineMonitor deadline_monitor{mock_monitor.as_deadline_monitor()};

    auto deadline_result{deadline_monitor.get_deadline(DeadlineTag("operation"))};
    ASSERT_TRUE(deadline_result.has_value());
    deadline::Deadline deadline{std::move(deadline_result.value())};

    auto handle_result{deadline.start()};
    ASSERT_TRUE(handle_result.has_value());
    handle_result->stop();
}

TEST_F(MockDeadlineMonitorFixture, UnregisteredDeadlineTag_ReturnsNotFound)
{
    RecordProperty("Description", "Verify get_deadline returns NotFound for an unregistered deadline tag.");

    MockDeadlineMonitor mock_monitor;
    mock_monitor.mock_deadline_for(DeadlineTag("operation"));

    deadline::DeadlineMonitor deadline_monitor{mock_monitor.as_deadline_monitor()};

    auto deadline_result{deadline_monitor.get_deadline(DeadlineTag("unregistered"))};
    ASSERT_FALSE(deadline_result.has_value());
    ASSERT_EQ(deadline_result.error(), Error::NotFound);
}
