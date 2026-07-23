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
#include "score/mw/health/logic_monitor.h"
#include "score/mw/health/mock/mock_logic_monitor.h"

#include <gtest/gtest.h>

using namespace score::mw::health;
using score::mw::health::logic::testing_support::MockLogicMonitor;
using ::testing::_;
using ::testing::Return;

/// Unit tests for the logic (state-machine) mock double (`MockLogicMonitor`), driven in isolation
/// through the real `LogicMonitor` -- without a `MockHealthMonitor`.
class MockLogicMonitorFixture : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        RecordProperty("TestType", "unit-test");
        RecordProperty("DerivationTechnique", "explorative-testing");
    }
};

TEST_F(MockLogicMonitorFixture, Transition_ForwardedThroughRealMonitor)
{
    RecordProperty(
        "Description",
        "Verify a real LogicMonitor can be driven through a mock standalone, without a MockHealthMonitor.");

    MockLogicMonitor logic_mock;
    EXPECT_CALL(logic_mock, transition(_)).WillOnce(Return(score::cpp::expected<StateTag, Error>(StateTag{"running"})));

    logic::LogicMonitor logic_monitor{logic_mock.as_logic_monitor()};

    auto transition_result{logic_monitor.transition(StateTag("running"))};
    ASSERT_TRUE(transition_result.has_value());
    ASSERT_EQ(transition_result.value(), StateTag("running"));
}

TEST_F(MockLogicMonitorFixture, TransitionAndState_ForwardedThroughRealMonitor)
{
    RecordProperty(
        "Description",
        "Verify MockLogicMonitor can simulate a state transition and report the resulting state through the real "
        "LogicMonitor.");

    MockLogicMonitor logic_mock;
    EXPECT_CALL(logic_mock, transition(_)).WillOnce(Return(score::cpp::expected<StateTag, Error>(StateTag{"stopped"})));
    EXPECT_CALL(logic_mock, state()).WillOnce(Return(score::cpp::expected<StateTag, Error>(StateTag{"stopped"})));

    logic::LogicMonitor logic_monitor{logic_mock.as_logic_monitor()};

    auto transition_result{logic_monitor.transition(StateTag("stopped"))};
    ASSERT_TRUE(transition_result.has_value());
    ASSERT_EQ(transition_result.value(), StateTag("stopped"));

    auto state_result{logic_monitor.state()};
    ASSERT_TRUE(state_result.has_value());
    ASSERT_EQ(state_result.value(), StateTag("stopped"));
}
