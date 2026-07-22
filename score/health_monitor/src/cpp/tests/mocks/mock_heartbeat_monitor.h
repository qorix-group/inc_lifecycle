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
#ifndef SCORE_HM_TESTS_MOCKS_MOCK_HEARTBEAT_MONITOR_H
#define SCORE_HM_TESTS_MOCKS_MOCK_HEARTBEAT_MONITOR_H

#include "score/mw/health/common.h"
#include <gmock/gmock.h>

namespace score::mw::health::heartbeat::testing_support
{

class MockHeartbeatMonitor
{
  public:
    MOCK_METHOD(void, Heartbeat, ());
};

}  // namespace score::mw::health::heartbeat::testing_support

#endif  // SCORE_HM_TESTS_MOCKS_MOCK_HEARTBEAT_MONITOR_H
