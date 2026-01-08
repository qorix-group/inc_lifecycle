// Copyright (c) 2025 Contributors to the Eclipse Foundation
//
// See the NOTICE file(s) distributed with this work for additional
// information regarding copyright ownership.
//
// This program and the accompanying materials are made available under the
// terms of the Apache License Version 2.0 which is available at
// <https://www.apache.org/licenses/LICENSE-2.0>
//
// SPDX-License-Identifier: Apache-2.0

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <ffi/logic_monitor.hpp>

int main()
{
    using namespace hm;

    LogicMonitorBuilder builder{LogicMonitorState("Initial")};
    auto monitor = builder
        .add_transition(LogicMonitorState("Initial"), LogicMonitorState("Running"))
        .add_transition(LogicMonitorState("Running"), LogicMonitorState("Paused"))
        .add_transition(LogicMonitorState("Paused"), LogicMonitorState("Running"))
        .add_transition(LogicMonitorState("Running"), LogicMonitorState("Stopped"))
        .build();
    assert(monitor.has_value());

    assert(monitor->status() == Status::Running);
    assert(monitor->disable() == Error::NoError);
    assert(monitor->status() == Status::Disabled);

    // Disable while disabled. Does nothing.
    assert(monitor->disable() == Error::NotAllowed);
    assert(monitor->status() == Status::Disabled);

    assert(monitor->enable() == Error::NoError);
    assert(monitor->status() == Status::Running);

    // Enable while enabled. Does nothing.
    assert(monitor->enable() == Error::NotAllowed);
    assert(monitor->status() == Status::Running);

    // Valid transition.
    assert(monitor->transition(LogicMonitorState("Running")) == Error::NoError);
    assert(monitor->state() == LogicMonitorState("Running"));
    assert(monitor->status() == Status::Running);

    // Valid transition.
    assert(monitor->transition(LogicMonitorState("Paused")) == Error::NoError);
    assert(monitor->state() == LogicMonitorState("Paused"));
    assert(monitor->status() == Status::Running);

    assert(monitor->disable() == Error::NoError);
    assert(monitor->status() == Status::Disabled);

    // Try a valid transition while disabled.
    assert(monitor->transition(LogicMonitorState("Running")) == Error::NotAllowed);
    assert(monitor->state() == LogicMonitorState("Paused"));
    assert(monitor->status() == Status::Disabled);

    // Try an invalid transition while disabled.
    assert(monitor->transition(LogicMonitorState("Stopped")) == Error::NotAllowed);
    assert(monitor->state() == LogicMonitorState("Paused"));
    assert(monitor->status() == Status::Disabled);

    assert(monitor->enable() == Error::NoError);
    assert(monitor->status() == Status::Running);

    // Try an invalid transition while enabled.
    assert(monitor->transition(LogicMonitorState("Stopped")) == Error::NotAllowed);
    assert(monitor->state() == LogicMonitorState("Paused"));
    assert(monitor->status() == Status::Failed);

    // Try to transition while failed.
    std::cout << "Trying to transition while failed" << std::endl;
    // No further state transitions should be printed.
    assert(monitor->transition(LogicMonitorState("Stopped")) == Error::Generic);
    assert(monitor->status() == Status::Failed);

    return EXIT_SUCCESS;
}
