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
#include "score/mw/health/heartbeat_monitor.h"
#include "score/mw/health/logic_monitor.h"
#include "score/health_monitor/src/cpp/tests/mocks/mock_health_monitor.h"

#include <gtest/gtest.h>
#include <optional>

using namespace score::mw::health;
using score::mw::health::deadline::testing_support::MockDeadline;
using score::mw::health::heartbeat::testing_support::MockHeartbeatMonitor;
using score::mw::health::logic::testing_support::MockLogicMonitor;
using score::mw::health::testing_support::MockHealthMonitor;
using ::testing::_;
using ::testing::Return;

/// Example application component that depends on the real `HealthMonitor` interface.
/// This demonstrates that user code can be tested with mocks alone, without linking the Rust FFI
/// library -- the mock is reached purely through the FFI boundary (see mock_health_monitor.h),
/// never by changing HealthMonitor/DeadlineMonitor/HeartbeatMonitor's public API or internals.
class ExampleComponent
{
  public:
    explicit ExampleComponent(HealthMonitor& health_monitor, MonitorTag heartbeat_tag, MonitorTag deadline_tag)
        : health_monitor_(health_monitor), heartbeat_tag_(heartbeat_tag), deadline_tag_(deadline_tag)
    {
    }

    bool Initialize()
    {
        auto hb_result{health_monitor_.get_heartbeat_monitor(heartbeat_tag_)};
        if (!hb_result.has_value())
        {
            return false;
        }
        heartbeat_monitor_ = std::move(hb_result.value());

        auto dl_result{health_monitor_.get_deadline_monitor(deadline_tag_)};
        if (!dl_result.has_value())
        {
            return false;
        }
        deadline_monitor_ = std::move(dl_result.value());

        return true;
    }

    void SendHeartbeat()
    {
        if (heartbeat_monitor_.has_value())
        {
            heartbeat_monitor_->heartbeat();
        }
    }

    bool RunTimedOperation()
    {
        if (!deadline_monitor_.has_value())
        {
            return false;
        }

        auto deadline_result{deadline_monitor_->get_deadline(DeadlineTag("operation"))};
        if (!deadline_result.has_value())
        {
            return false;
        }

        auto handle_result{deadline_result.value().start()};
        if (!handle_result.has_value())
        {
            return false;
        }
        handle_result->stop();

        return true;
    }

  private:
    HealthMonitor& health_monitor_;
    MonitorTag heartbeat_tag_;
    MonitorTag deadline_tag_;
    std::optional<heartbeat::HeartbeatMonitor> heartbeat_monitor_;
    std::optional<deadline::DeadlineMonitor> deadline_monitor_;
};

namespace
{
/// `mock` must outlive the returned HealthMonitor -- it is what the returned object is pinned to.
HealthMonitor BuildHealthMonitorWith(MockHealthMonitor& mock, bool with_heartbeat, bool with_deadline)
{
    if (with_heartbeat)
    {
        mock.MockHeartbeatMonitorFor(MonitorTag("heartbeat"));
    }
    if (with_deadline)
    {
        mock.MockDeadlineMonitorFor(MonitorTag("deadline"));
    }
    return mock.AsHealthMonitor();
}
}  // namespace

class ExampleComponentTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        RecordProperty("TestType", "unit-test");
        RecordProperty("DerivationTechnique", "explorative-testing");
    }
};

TEST_F(ExampleComponentTest, Initialization_WithMocks)
{
    RecordProperty("Description",
                   "Verify that user code can be initialized and tested against real HealthMonitor "
                   "types backed by mocks, without the Rust FFI library.");

    MockHealthMonitor mock;
    HealthMonitor health_monitor{BuildHealthMonitorWith(mock, /*with_heartbeat=*/true, /*with_deadline=*/true)};

    ExampleComponent component(health_monitor, MonitorTag("heartbeat"), MonitorTag("deadline"));
    ASSERT_TRUE(component.Initialize());
}

TEST_F(ExampleComponentTest, Heartbeat_WithMocks)
{
    RecordProperty("Description", "Verify heartbeat is forwarded through the real HeartbeatMonitor to its mock.");

    MockHealthMonitor mock;
    HealthMonitor health_monitor{BuildHealthMonitorWith(mock, /*with_heartbeat=*/true, /*with_deadline=*/true)};

    ExampleComponent component(health_monitor, MonitorTag("heartbeat"), MonitorTag("deadline"));
    ASSERT_TRUE(component.Initialize());

    MockHeartbeatMonitor& heartbeat_mock{mock.MockHeartbeatMonitorFor(MonitorTag("heartbeat"))};
    EXPECT_CALL(heartbeat_mock, Heartbeat()).Times(1);

    component.SendHeartbeat();
}

TEST_F(ExampleComponentTest, TimedOperation_WithMocks)
{
    RecordProperty("Description", "Verify deadline start/stop is forwarded through the real Deadline to its mock.");

    MockHealthMonitor mock;
    HealthMonitor health_monitor{BuildHealthMonitorWith(mock, /*with_heartbeat=*/true, /*with_deadline=*/true)};

    ExampleComponent component(health_monitor, MonitorTag("heartbeat"), MonitorTag("deadline"));
    ASSERT_TRUE(component.Initialize());

    MockDeadline& deadline_mock{mock.MockDeadlineMonitorFor(MonitorTag("deadline")).MockDeadlineFor(DeadlineTag("operation"))};
    {
        ::testing::InSequence in_order;
        EXPECT_CALL(deadline_mock, Start()).WillOnce(Return(internal::kSuccess));
        EXPECT_CALL(deadline_mock, Stop()).WillOnce(Return(internal::kSuccess));
        EXPECT_CALL(deadline_mock, Destroy()).WillOnce(Return(internal::kSuccess));
    }

    ASSERT_TRUE(component.RunTimedOperation());
}

TEST_F(ExampleComponentTest, Initialization_FailsOnMissingMonitor)
{
    RecordProperty("Description", "Verify component handles initialization failure gracefully.");

    // Deliberately register only a deadline monitor -- "heartbeat" is never added, so
    // get_heartbeat_monitor() below resolves to Error::NotFound.
    MockHealthMonitor mock;
    HealthMonitor health_monitor{BuildHealthMonitorWith(mock, /*with_heartbeat=*/false, /*with_deadline=*/true)};

    ExampleComponent component(health_monitor, MonitorTag("heartbeat"), MonitorTag("deadline"));
    ASSERT_FALSE(component.Initialize());
}

TEST_F(ExampleComponentTest, LogicMonitorMock_Transition)
{
    RecordProperty("Description", "Verify LogicMonitorMock can simulate state transitions through the real LogicMonitor.");

    MockHealthMonitor mock;
    MockLogicMonitor& logic_mock{mock.MockLogicMonitorFor(MonitorTag("logic"))};
    HealthMonitor health_monitor{mock.AsHealthMonitor()};

    EXPECT_CALL(logic_mock, Transition(_)).WillOnce(Return(score::cpp::expected<StateTag, Error>(StateTag{"stopped"})));
    EXPECT_CALL(logic_mock, State()).WillOnce(Return(score::cpp::expected<StateTag, Error>(StateTag{"stopped"})));

    auto logic_monitor_result{health_monitor.get_logic_monitor(MonitorTag("logic"))};
    ASSERT_TRUE(logic_monitor_result.has_value());
    logic::LogicMonitor logic_monitor{std::move(logic_monitor_result.value())};

    auto transition_result{logic_monitor.transition(StateTag("stopped"))};
    ASSERT_TRUE(transition_result.has_value());
    ASSERT_EQ(transition_result.value(), StateTag("stopped"));

    auto state_result{logic_monitor.state()};
    ASSERT_TRUE(state_result.has_value());
    ASSERT_EQ(state_result.value(), StateTag("stopped"));
}
