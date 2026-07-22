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
#ifndef SCORE_HM_TESTS_MOCKS_MOCK_DEADLINE_MONITOR_H
#define SCORE_HM_TESTS_MOCKS_MOCK_DEADLINE_MONITOR_H

#include "mock_deadline.h"
#include "score/mw/health/deadline_monitor.h"
#include <map>
#include <memory>

namespace score::mw::health::deadline::testing_support
{

/// Owns the MockDeadline instances handed out for each DeadlineTag, and can produce a real,
/// production `DeadlineMonitor` bound to itself via `AsDeadlineMonitor()`.
class MockDeadlineMonitor
{
  public:
    /// Get-or-create the MockDeadline registered for `deadline_tag`, to set expectations on.
    MockDeadline& MockDeadlineFor(const DeadlineTag& deadline_tag)
    {
        return *deadlines_.emplace(deadline_tag, std::make_unique<MockDeadline>()).first->second;
    }

    /// Used by mock_deadline_ffi.cpp to route `deadline_monitor_get_deadline`.
    MockDeadline* FindDeadline(const DeadlineTag& deadline_tag)
    {
        auto it{deadlines_.find(deadline_tag)};
        return it == deadlines_.end() ? nullptr : it->second.get();
    }

    /// Real production `Deadline`, pinned to the (get-or-create) MockDeadline for `deadline_tag`.
    Deadline GetDeadline(const DeadlineTag& deadline_tag)
    {
        MockDeadlineFor(deadline_tag);
        auto monitor{AsDeadlineMonitor()};
        auto result{monitor.get_deadline(deadline_tag)};
        return std::move(result.value());
    }

    /// Real production `DeadlineMonitor` whose FFIHandle is this mock's own address.
    DeadlineMonitor AsDeadlineMonitor()
    {
        return internal::ConstructibleFrom<DeadlineMonitor>::create(reinterpret_cast<internal::FFIHandle>(this));
    }

  private:
    std::map<DeadlineTag, std::unique_ptr<MockDeadline>> deadlines_;
};

}  // namespace score::mw::health::deadline::testing_support

#endif  // SCORE_HM_TESTS_MOCKS_MOCK_DEADLINE_MONITOR_H
