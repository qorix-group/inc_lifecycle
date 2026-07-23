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
#include "score/mw/health/health_monitor.h"
#include "score/mw/health/heartbeat_monitor.h"
#include "score/mw/health/logic_monitor.h"
#include "score/mw/health/mock/mock_health_monitor.h"

#include <gtest/gtest.h>
#include <optional>
#include <utility>

using namespace score::mw::health;
using score::mw::health::deadline::testing_support::MockDeadline;
using score::mw::health::heartbeat::testing_support::MockHeartbeatMonitor;
using score::mw::health::logic::testing_support::MockLogicMonitor;
using score::mw::health::testing_support::MockHealthMonitor;
using ::testing::Return;

/// Example application component that depends on the real `HealthMonitor` interface and exercises
/// all three monitor families -- heartbeat, deadline, and logic (state machine). It demonstrates
/// that a non-trivial user component can be integration-tested against real `HealthMonitor` types
/// backed by a single `MockHealthMonitor`, without linking the Rust FFI library -- the mocks are
/// reached purely through the FFI boundary (see mock_health_monitor.h), never by changing the
/// monitors' public API or internals.
class ExampleComponent
{
  public:
    ExampleComponent(
        HealthMonitor& health_monitor,
        MonitorTag heartbeat_tag,
        MonitorTag deadline_tag,
        MonitorTag logic_tag)
        : health_monitor_(health_monitor),
          heartbeat_tag_(heartbeat_tag),
          deadline_tag_(deadline_tag),
          logic_tag_(logic_tag)
    {
    }

    bool initialize()
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

        auto lg_result{health_monitor_.get_logic_monitor(logic_tag_)};
        if (!lg_result.has_value())
        {
            return false;
        }
        logic_monitor_ = std::move(lg_result.value());

        return true;
    }

    bool start()
    {
        if (!logic_monitor_.has_value())
        {
            return false;
        }
        return logic_monitor_->transition(StateTag("running")).has_value();
    }

    bool stop()
    {
        if (!logic_monitor_.has_value())
        {
            return false;
        }
        return logic_monitor_->transition(StateTag("stopped")).has_value();
    }

    void send_heartbeat()
    {
        if (heartbeat_monitor_.has_value())
        {
            heartbeat_monitor_->heartbeat();
        }
    }

    bool run_timed_operation()
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
    MonitorTag logic_tag_;
    std::optional<heartbeat::HeartbeatMonitor> heartbeat_monitor_;
    std::optional<deadline::DeadlineMonitor> deadline_monitor_;
    std::optional<logic::LogicMonitor> logic_monitor_;
};

/// Integration fixture: wires a single `MockHealthMonitor` with the requested monitor family mocks
/// and pins a real `HealthMonitor` to it, showing how the whole component is tested against one
/// larger mock.
class ExampleComponentFixture : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        RecordProperty("TestType", "integration-test");
        RecordProperty("DerivationTechnique", "explorative-testing");
    }

    /// Register the requested monitors on `mock_` and return a real HealthMonitor pinned to it.
    /// `mock_` must outlive the returned HealthMonitor -- it is what the returned object is pinned to.
    HealthMonitor build_health_monitor(bool with_heartbeat, bool with_deadline, bool with_logic)
    {
        if (with_heartbeat)
        {
            mock_.mock_heartbeat_monitor_for(heartbeat_tag_);
        }
        if (with_deadline)
        {
            mock_.mock_deadline_monitor_for(deadline_tag_);
        }
        if (with_logic)
        {
            mock_.mock_logic_monitor_for(logic_tag_);
        }
        return mock_.as_health_monitor();
    }

    const MonitorTag heartbeat_tag_{"heartbeat"};
    const MonitorTag deadline_tag_{"deadline"};
    const MonitorTag logic_tag_{"logic"};
    MockHealthMonitor mock_;
};

TEST_F(ExampleComponentFixture, Initialization_WithMocks)
{
    RecordProperty(
        "Description",
        "Verify the component can be initialized against real HealthMonitor types backed by mocks, "
        "without the Rust FFI library.");

    HealthMonitor health_monitor{
        build_health_monitor(/*with_heartbeat=*/true, /*with_deadline=*/true, /*with_logic=*/true)};

    ExampleComponent component(health_monitor, heartbeat_tag_, deadline_tag_, logic_tag_);
    ASSERT_TRUE(component.initialize());
}

TEST_F(ExampleComponentFixture, Heartbeat_WithMocks)
{
    RecordProperty("Description", "Verify heartbeat is forwarded through the real HeartbeatMonitor to its mock.");

    HealthMonitor health_monitor{
        build_health_monitor(/*with_heartbeat=*/true, /*with_deadline=*/true, /*with_logic=*/true)};

    ExampleComponent component(health_monitor, heartbeat_tag_, deadline_tag_, logic_tag_);
    ASSERT_TRUE(component.initialize());

    MockHeartbeatMonitor& heartbeat_mock{mock_.mock_heartbeat_monitor_for(heartbeat_tag_)};
    EXPECT_CALL(heartbeat_mock, heartbeat()).Times(1);

    component.send_heartbeat();
}

TEST_F(ExampleComponentFixture, TimedOperation_WithMocks)
{
    RecordProperty("Description", "Verify deadline start/stop is forwarded through the real Deadline to its mock.");

    HealthMonitor health_monitor{
        build_health_monitor(/*with_heartbeat=*/true, /*with_deadline=*/true, /*with_logic=*/true)};

    ExampleComponent component(health_monitor, heartbeat_tag_, deadline_tag_, logic_tag_);
    ASSERT_TRUE(component.initialize());

    MockDeadline& deadline_mock{
        mock_.mock_deadline_monitor_for(deadline_tag_).mock_deadline_for(DeadlineTag("operation"))};
    {
        ::testing::InSequence in_order;
        EXPECT_CALL(deadline_mock, start()).WillOnce(Return(internal::kSuccess));
        EXPECT_CALL(deadline_mock, stop()).WillOnce(Return(internal::kSuccess));
        EXPECT_CALL(deadline_mock, destroy()).WillOnce(Return(internal::kSuccess));
    }

    ASSERT_TRUE(component.run_timed_operation());
}

TEST_F(ExampleComponentFixture, StateTransitions_WithMocks)
{
    RecordProperty("Description", "Verify start/stop drive the real LogicMonitor state machine through its mock.");

    HealthMonitor health_monitor{
        build_health_monitor(/*with_heartbeat=*/true, /*with_deadline=*/true, /*with_logic=*/true)};

    ExampleComponent component(health_monitor, heartbeat_tag_, deadline_tag_, logic_tag_);
    ASSERT_TRUE(component.initialize());

    MockLogicMonitor& logic_mock{mock_.mock_logic_monitor_for(logic_tag_)};
    {
        ::testing::InSequence in_order;
        EXPECT_CALL(logic_mock, transition(StateTag("running")))
            .WillOnce(Return(score::cpp::expected<StateTag, Error>(StateTag{"running"})));
        EXPECT_CALL(logic_mock, transition(StateTag("stopped")))
            .WillOnce(Return(score::cpp::expected<StateTag, Error>(StateTag{"stopped"})));
    }

    ASSERT_TRUE(component.start());
    ASSERT_TRUE(component.stop());
}

TEST_F(ExampleComponentFixture, FullLifecycle_WithAllMonitors)
{
    RecordProperty(
        "Description",
        "Verify a full component lifecycle -- start, heartbeat, timed operation, stop -- drives all three monitor "
        "families in order through a single MockHealthMonitor.");

    HealthMonitor health_monitor{
        build_health_monitor(/*with_heartbeat=*/true, /*with_deadline=*/true, /*with_logic=*/true)};

    ExampleComponent component(health_monitor, heartbeat_tag_, deadline_tag_, logic_tag_);
    ASSERT_TRUE(component.initialize());

    MockHeartbeatMonitor& heartbeat_mock{mock_.mock_heartbeat_monitor_for(heartbeat_tag_)};
    MockDeadline& deadline_mock{
        mock_.mock_deadline_monitor_for(deadline_tag_).mock_deadline_for(DeadlineTag("operation"))};
    MockLogicMonitor& logic_mock{mock_.mock_logic_monitor_for(logic_tag_)};
    {
        ::testing::InSequence in_order;
        EXPECT_CALL(logic_mock, transition(StateTag("running")))
            .WillOnce(Return(score::cpp::expected<StateTag, Error>(StateTag{"running"})));
        EXPECT_CALL(heartbeat_mock, heartbeat()).Times(1);
        EXPECT_CALL(deadline_mock, start()).WillOnce(Return(internal::kSuccess));
        EXPECT_CALL(deadline_mock, stop()).WillOnce(Return(internal::kSuccess));
        EXPECT_CALL(deadline_mock, destroy()).WillOnce(Return(internal::kSuccess));
        EXPECT_CALL(logic_mock, transition(StateTag("stopped")))
            .WillOnce(Return(score::cpp::expected<StateTag, Error>(StateTag{"stopped"})));
    }

    ASSERT_TRUE(component.start());
    component.send_heartbeat();
    ASSERT_TRUE(component.run_timed_operation());
    ASSERT_TRUE(component.stop());
}

TEST_F(ExampleComponentFixture, Initialization_FailsOnMissingMonitor)
{
    RecordProperty("Description", "Verify component handles initialization failure gracefully.");

    // Deliberately omit the heartbeat monitor -- get_heartbeat_monitor() resolves to Error::NotFound.
    HealthMonitor health_monitor{
        build_health_monitor(/*with_heartbeat=*/false, /*with_deadline=*/true, /*with_logic=*/true)};

    ExampleComponent component(health_monitor, heartbeat_tag_, deadline_tag_, logic_tag_);
    ASSERT_FALSE(component.initialize());
}
