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

/// @file
/// @brief Unit test of the PLMS ReportRunningImpl against the real QNX name space.
///
/// ReportRunningImpl::reported is process-wide and set only on success, so one test runs every case in a fixed order.
/// @note The attachment made by the successful case stays until the test process exits.

#include <gtest/gtest.h>

#include <sys/dispatch.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <string>

#include "score/mw/lifecycle/execution_error.h"
#include "score/mw/lifecycle/lifecycle_client/details/report_running_impl.hpp"

namespace score::mw::lifecycle
{
namespace
{

constexpr const char* kReadyNameVariable{"LIFECYCLE_READY_NAME"};

bool IsAttached(const std::string& name)
{
    struct stat info{};
    return ::stat(("/dev/name/local/" + name).c_str(), &info) == 0;
}

/// @brief Child side of case (1): attaches `name`, writes 1 or 0 to `to_parent`, and holds it until `from_parent`
/// is written or closed.
[[noreturn]] void HoldNameInChild(const std::string& name, const int to_parent, const int from_parent)
{
    name_attach_t* const attach = ::name_attach(nullptr, name.c_str(), 0U);
    const char attached = (attach != nullptr) ? 1 : 0;
    static_cast<void>(::write(to_parent, &attached, 1U));
    char release{};
    static_cast<void>(::read(from_parent, &release, 1U));
    // Detaching here rather than relying on process teardown frees the name before the parent's waitpid() returns.
    if (attach != nullptr)
    {
        static_cast<void>(::name_detach(attach, 0U));
    }

    ::_exit(0);
}

TEST(ReportRunningImplPlmsUT, AttachesReadyNameOnceAndOnlyOnSuccess)
{
    RecordProperty("TestType", "interface-test");
    RecordProperty("DerivationTechnique", "equivalence-classes");
    RecordProperty(
        "Description",
        "ReportRunningState fails with kCommunicationError when the ready name is taken, unset or empty, "
        "succeeds by attaching it, and fails with kInvalidTransition on a second call.");

    const std::string name{"lifecycle_plms_ut_" + std::to_string(::getpid())};
    const ReportRunningImpl impl{};

    // (1) Name already attached by another process.
    {
        int to_parent[2]{};
        int to_child[2]{};
        ASSERT_EQ(::pipe(to_parent), 0);
        ASSERT_EQ(::pipe(to_child), 0);
        const pid_t child = ::fork();
        ASSERT_NE(child, -1);
        if (child == 0)
        {
            ::close(to_parent[0]);
            ::close(to_child[1]);
            HoldNameInChild(name, to_parent[1], to_child[0]);
        }

        ::close(to_parent[1]);
        ::close(to_child[0]);

        char attached{0};
        ASSERT_EQ(::read(to_parent[0], &attached, 1U), 1);
        ASSERT_EQ(attached, 1);
        ASSERT_EQ(::setenv(kReadyNameVariable, name.c_str(), 1), 0);

        const auto result = impl.ReportRunningState();

        // A success here would set `reported` and invalidate every later case.
        ASSERT_FALSE(result.has_value());
        EXPECT_EQ(result.error(), ExecErrc::kCommunicationError);

        ::close(to_child[1]);
        int status{0};
        ASSERT_EQ(::waitpid(child, &status, 0), child);
        EXPECT_TRUE(WIFEXITED(status));
        ::close(to_parent[0]);
        ASSERT_FALSE(IsAttached(name));
    }

    // (2) LIFECYCLE_READY_NAME unset.
    {
        ASSERT_EQ(::unsetenv(kReadyNameVariable), 0);

        const auto result = impl.ReportRunningState();

        ASSERT_FALSE(result.has_value());
        EXPECT_EQ(result.error(), ExecErrc::kCommunicationError);
        EXPECT_FALSE(IsAttached(name));
    }

    // (3) LIFECYCLE_READY_NAME empty.
    {
        ASSERT_EQ(::setenv(kReadyNameVariable, "", 1), 0);

        const auto result = impl.ReportRunningState();

        ASSERT_FALSE(result.has_value());
        EXPECT_EQ(result.error(), ExecErrc::kCommunicationError);
        EXPECT_FALSE(IsAttached(name));
    }

    // (4) LIFECYCLE_READY_NAME set; the failures above must not have marked the process as reported.
    {
        ASSERT_EQ(::setenv(kReadyNameVariable, name.c_str(), 1), 0);

        const auto result = impl.ReportRunningState();

        ASSERT_TRUE(result.has_value());
        EXPECT_TRUE(IsAttached(name));
    }

    // (5) Second report after success.
    {
        const auto result = impl.ReportRunningState();

        ASSERT_FALSE(result.has_value());
        EXPECT_EQ(result.error(), ExecErrc::kInvalidTransition);
        EXPECT_TRUE(IsAttached(name));
    }
}

}  // namespace
}  // namespace score::mw::lifecycle
