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
/// @brief ReportRunningImpl for a process started by QNX PLMS, selected with lifecycle_backend=plms.
///
/// PLMS has no client call for readiness; it observes it with waitfor_attachname() in the component's start sequence.
/// Reporting Running therefore attaches the name in LIFECYCLE_READY_NAME, which the PLMS configuration sets for the
/// component and waits for as /dev/name/local/<name>.
/// @pre The name in LIFECYCLE_READY_NAME is unique on the node; a second process attaching it fails the report.
/// @note The attachment lasts until the process exits; the OS removes it then.

#include <sys/dispatch.h>

#include <atomic>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <variant>

#include "score/result/result.h"

#include "score/mw/launch_manager/common/log.hpp"
#include "score/mw/lifecycle/execution_error.h"
#include "score/mw/lifecycle/lifecycle_client/details/report_running_impl.hpp"

namespace score::mw::lifecycle
{

namespace
{

constexpr const char* kReadyNameVariable{"LIFECYCLE_READY_NAME"};

}  // namespace

std::atomic_bool ReportRunningImpl::reported{false};

ReportRunningImpl::ReportRunningImpl() noexcept = default;

ReportRunningImpl::~ReportRunningImpl() noexcept = default;

score::Result<std::monostate> ReportRunningImpl::ReportRunningState() const noexcept
{
    if (reported)
    {
        LM_LOG_INFO() << "[Lifecycle Client] Reported Running state already!";
        return score::MakeUnexpected(ExecErrc::kInvalidTransition);
    }

    return reportKRunningtoDaemon();
}

score::Result<std::monostate> ReportRunningImpl::reportKRunningtoDaemon() const noexcept
{
    const char* const ready_name = std::getenv(kReadyNameVariable);
    if ((ready_name == nullptr) || (ready_name[0] == '\0'))
    {
        LM_LOG_ERROR() << "[Lifecycle Client] " << kReadyNameVariable
                       << " is not set, so this process was not started by PLMS with a ready name";
        return score::MakeUnexpected(ExecErrc::kCommunicationError);
    }

    // The handle is never detached: the attached name is the readiness PLMS observes for as long as the process runs.
    name_attach_t* const attach = name_attach(nullptr, ready_name, 0U);
    if (attach == nullptr)
    {
        LM_LOG_ERROR() << "[Lifecycle Client] name_attach(" << ready_name << ") failed: " << std::strerror(errno);
        return score::MakeUnexpected(ExecErrc::kCommunicationError);
    }

    reported = true;
    return score::Result<std::monostate>{};
}

}  // namespace score::mw::lifecycle
