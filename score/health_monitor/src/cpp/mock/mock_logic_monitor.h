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
#ifndef SCORE_HM_TESTS_MOCKS_MOCK_LOGIC_MONITOR_H
#define SCORE_HM_TESTS_MOCKS_MOCK_LOGIC_MONITOR_H

#include "score/mw/health/common.h"
#include "score/mw/health/logic_monitor.h"
#include "score/mw/health/tag.h"
#include "testing_factory.h"
#include <gmock/gmock.h>
#include <score/expected.hpp>

namespace score::mw::health::logic::testing_support
{

class MockLogicMonitor
{
  public:
    MOCK_METHOD((score::cpp::expected<StateTag, Error>), transition, (const StateTag& state));
    MOCK_METHOD((score::cpp::expected<StateTag, Error>), state, ());

    /// Real production `LogicMonitor` whose FFIHandle is this mock's own address.
    LogicMonitor as_logic_monitor()
    {
        return internal::ConstructibleFrom<LogicMonitor>::create(reinterpret_cast<internal::FFIHandle>(this));
    }
};

}  // namespace score::mw::health::logic::testing_support

#endif  // SCORE_HM_TESTS_MOCKS_MOCK_LOGIC_MONITOR_H
