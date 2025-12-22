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
//

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>
#include <ffi/deadline_monitor.hpp>

std::mutex on_status_changed_lock;

int main()
{
    auto *builder = hm_dmb_new();

    hm_DeadlineMonitor *monitor;
    hm_dmb_build(&builder, &monitor);

    assert(builder == nullptr);
    assert(hm_dm_status(monitor) == hm_Status::Running);

    hm_dm_disable(monitor);
    assert(hm_dm_status(monitor) == hm_Status::Disabled);

    // Disable while disabled. Does nothing.
    hm_dm_disable(monitor);  // TODO: Should report error and the error should be handled.
    assert(hm_dm_status(monitor) == hm_Status::Disabled);

    hm_dm_enable(monitor);
    assert(hm_dm_status(monitor) == hm_Status::Running);

    // Enable while enabled. Does nothing.
    hm_dm_enable(monitor);  // TODO: Should report error and the error should be handled.
    assert(hm_dm_status(monitor) == hm_Status::Running);

    using namespace std::chrono_literals;

    std::thread t1(
        [&]()
        {
            hm_Deadline *deadline1;
            hm_dm_create_custom_deadline(monitor, 10, 1000, &deadline1);
            hm_Deadline *deadline2;
            hm_dm_create_custom_deadline(monitor, 50, 250, &deadline2);

            // Run task 1.
            hm_dl_start(deadline1);
            std::this_thread::sleep_for(250ms);
            hm_dl_stop(deadline1);

            assert(hm_dm_status(monitor) == hm_Status::Running);

            // Run task 2.
            hm_dl_start(deadline2);
            std::this_thread::sleep_for(100ms);
            hm_dl_stop(deadline2);

            assert(hm_dm_status(monitor) == hm_Status::Running);

            // Run task 1 again.
            hm_dl_start(deadline1);
            std::this_thread::sleep_for(250ms);
            hm_dl_stop(deadline1);

            assert(hm_dm_status(monitor) == hm_Status::Running);

            hm_dl_delete(&deadline1);
            assert(deadline1 == nullptr);
            hm_dl_delete(&deadline2);
            assert(deadline2 == nullptr);
        });

    t1.join();

    std::thread t2(
        [&]()
        {
            hm_Deadline *deadline;
            hm_dm_create_custom_deadline(monitor, 0, 100, &deadline);

            // This task is too long.
            hm_dl_start(deadline);
            std::this_thread::sleep_for(250ms);
            hm_dl_stop(deadline);

            assert(hm_dm_status(monitor) == hm_Status::Failed);

            hm_dl_delete(&deadline);
            assert(deadline == nullptr);
        });

    t2.join();

    assert(hm_dm_status(monitor) == hm_Status::Failed);

    hm_dm_delete(&monitor);
    assert(monitor == nullptr);

    return EXIT_SUCCESS;
}
