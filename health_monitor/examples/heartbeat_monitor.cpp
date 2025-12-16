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

using namespace std::chrono_literals;

int main()
{
    auto heartbeat_monitor = hm_hbm_new(500U);
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

    std::thread t2(
        [&]()
        {
            hm_Status status = hm_Status::Running;
            while (status == hm_Status::Running)
            {
                status = hm_hbm_status(heartbeat_monitor);
                std::this_thread::sleep_for(100ms);
            }
            std::cout << "heartbeat monitoring failed, status: " << static_cast<int32_t>(status)
                      << std::endl;
        });

    t2.join();

    hm_hbm_delete(&heartbeat_monitor);
    assert(heartbeat_monitor == nullptr);

    return EXIT_SUCCESS;
}
