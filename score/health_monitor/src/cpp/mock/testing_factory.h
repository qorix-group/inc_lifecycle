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
#ifndef SCORE_HM_TESTS_MOCKS_TESTING_FACTORY_H
#define SCORE_HM_TESTS_MOCKS_TESTING_FACTORY_H

#include "score/mw/health/common.h"
#include <utility>

namespace score::mw::health::internal
{

/// Test-only factory that constructs a production type through its private constructor.
/// Production types grant access by befriending `internal::ConstructibleFrom<T>`.
template <typename T>
class ConstructibleFrom
{
  public:
    template <typename... Args>
    static T create(Args&&... args)
    {
        return T(std::forward<Args>(args)...);
    }
};

/// Non-null placeholder handle for FFI shims whose handle value is never dereferenced.
/// Must be non-null: `DroppableFFIHandle` treats a nullptr handle as "already dropped".
inline FFIHandle non_null_handle_sentinel()
{
    return reinterpret_cast<FFIHandle>(1);
}

}  // namespace score::mw::health::internal

#endif  // SCORE_HM_TESTS_MOCKS_TESTING_FACTORY_H
