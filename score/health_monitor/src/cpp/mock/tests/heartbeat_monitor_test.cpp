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
#include "score/mw/health/heartbeat_monitor.h"
#include "score/mw/health/mock/mock_heartbeat_monitor.h"

#include <gtest/gtest.h>

using namespace score::mw::health;
using score::mw::health::heartbeat::testing_support::MockHeartbeatMonitor;

/// Unit tests for the heartbeat mock double (`MockHeartbeatMonitor`), driven in isolation through
/// the real `HeartbeatMonitor` -- without a `MockHealthMonitor`.
class MockHeartbeatMonitorFixture : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        RecordProperty("TestType", "unit-test");
        RecordProperty("DerivationTechnique", "explorative-testing");
    }
};

TEST_F(MockHeartbeatMonitorFixture, Heartbeat_ForwardedThroughRealMonitor)
{
    RecordProperty(
        "Description",
        "Verify a real HeartbeatMonitor can be driven through a mock standalone, without a MockHealthMonitor.");

    MockHeartbeatMonitor heartbeat_mock;
    EXPECT_CALL(heartbeat_mock, heartbeat()).Times(1);

    heartbeat::HeartbeatMonitor heartbeat_monitor{heartbeat_mock.as_heartbeat_monitor()};
    heartbeat_monitor.heartbeat();
}
