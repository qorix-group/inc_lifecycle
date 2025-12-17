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
#include <ffi.h>

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>

#include "../alive_monitor/include/alive_monitor.h"

using namespace std::chrono_literals;

int main()
{
    auto heartbeat_monitor = hm_hbm_new(500U);
    auto deadline_monitor_builder = hm_dmb_new();
    auto deadline_monitor = hm_dmb_build(&deadline_monitor_builder);

    auto logic_monitor_builder = hm_lmb_new(hm_lm_state_from_str("Initial"));
    hm_lmb_add_transition(logic_monitor_builder, hm_lm_state_from_str("Initial"),
                          hm_lm_state_from_str("Running"));
    hm_lmb_add_transition(logic_monitor_builder, hm_lm_state_from_str("Running"),
                          hm_lm_state_from_str("Paused"));
    hm_lmb_add_transition(logic_monitor_builder, hm_lm_state_from_str("Paused"),
                          hm_lm_state_from_str("Running"));
    hm_lmb_add_transition(logic_monitor_builder, hm_lm_state_from_str("Running"),
                          hm_lm_state_from_str("Stopped"));
    auto logic_monitor = hm_lmb_build(&logic_monitor_builder);
    hm_lm_disable(logic_monitor);
    hm_dm_disable(deadline_monitor);

    auto minimum_time_ms = std::chrono::milliseconds{500};
    auto alive_monitor = AliveMonitor(minimum_time_ms);

    auto health_monitor = hm_new(deadline_monitor, logic_monitor, heartbeat_monitor,
                                 alive_monitor.Release(), minimum_time_ms.count());

    std::thread t1(
        [&]()
        {
            uint32_t iteration = 0U;
            while (true)
            {
                if (iteration < 10)
                {
                    hm_hbm_send_heartbeat(heartbeat_monitor);
                    std::this_thread::sleep_for(250ms);
                    std::cout << "Heartbeat sent!" << std::endl;
                }
                else
                {
                    std::cout << "Simulating thread misbehaving..." << std::endl;
                    break;
                }
                ++iteration;
            }
        });

    t1.join();

    sleep(2);
    hm_delete(&health_monitor);
    assert(health_monitor == nullptr);

    return EXIT_SUCCESS;
}
