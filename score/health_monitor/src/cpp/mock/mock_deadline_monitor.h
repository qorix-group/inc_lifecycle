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
#include "testing_factory.h"
#include <memory>
#include <utility>
#include <vector>

namespace score::mw::health::deadline::testing_support
{

/// Owns the MockDeadline instances handed out for each DeadlineTag, and can produce a real,
/// production `DeadlineMonitor` bound to itself via `as_deadline_monitor()`.
class MockDeadlineMonitor
{
  public:
    /// Get-or-create the MockDeadline registered for `deadline_tag`, to set expectations on.
    MockDeadline& mock_deadline_for(const DeadlineTag& deadline_tag)
    {
        if (auto* existing{find_deadline(deadline_tag)}; existing != nullptr)
        {
            return *existing;
        }
        deadlines_.emplace_back(deadline_tag, std::make_unique<MockDeadline>());
        return *deadlines_.back().second;
    }

    /// Used by mock_deadline_monitor_ffi.cpp to route `deadline_monitor_get_deadline`.
    MockDeadline* find_deadline(const DeadlineTag& deadline_tag)
    {
        for (auto& [tag, mock] : deadlines_)
        {
            if (tag == deadline_tag)
            {
                return mock.get();
            }
        }
        return nullptr;
    }

    /// Real production `Deadline`, pinned to the (get-or-create) MockDeadline for `deadline_tag`.
    Deadline get_deadline(const DeadlineTag& deadline_tag)
    {
        mock_deadline_for(deadline_tag);
        auto monitor{as_deadline_monitor()};
        auto result{monitor.get_deadline(deadline_tag)};
        return std::move(result.value());
    }

    /// Real production `DeadlineMonitor` whose FFIHandle is this mock's own address.
    DeadlineMonitor as_deadline_monitor()
    {
        return internal::ConstructibleFrom<DeadlineMonitor>::create(reinterpret_cast<internal::FFIHandle>(this));
    }

  private:
    std::vector<std::pair<DeadlineTag, std::unique_ptr<MockDeadline>>> deadlines_;
};

}  // namespace score::mw::health::deadline::testing_support

#endif  // SCORE_HM_TESTS_MOCKS_MOCK_DEADLINE_MONITOR_H
