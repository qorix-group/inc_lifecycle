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
#include "score/mw/health/tag.h"
#include <gmock/gmock.h>
#include <score/expected.hpp>

namespace score::mw::health::logic::testing_support
{

class MockLogicMonitor
{
  public:
    MOCK_METHOD((score::cpp::expected<StateTag, Error>), Transition, (const StateTag& state));
    MOCK_METHOD((score::cpp::expected<StateTag, Error>), State, ());
};

}  // namespace score::mw::health::logic::testing_support

#endif  // SCORE_HM_TESTS_MOCKS_MOCK_LOGIC_MONITOR_H
