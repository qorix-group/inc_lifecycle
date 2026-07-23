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
#ifndef SCORE_HM_TESTS_MOCKS_MOCK_DEADLINE_H
#define SCORE_HM_TESTS_MOCKS_MOCK_DEADLINE_H

#include "score/mw/health/common.h"
#include <gmock/gmock.h>

namespace score::mw::health::deadline::testing_support
{

class MockDeadline
{
  public:
    MOCK_METHOD(internal::FFICode, start, ());
    MOCK_METHOD(internal::FFICode, stop, ());
    MOCK_METHOD(internal::FFICode, destroy, ());
};

}  // namespace score::mw::health::deadline::testing_support

#endif  // SCORE_HM_TESTS_MOCKS_MOCK_DEADLINE_H
