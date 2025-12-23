/********************************************************************************
 * Copyright (c) 2025 Contributors to the Eclipse Foundation
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
#include <iostream>

#include "ffi.h"

int main()
{
    auto heartbeat_monitor = hm_hbm_new(100U, 500U);
    auto result = hm_hbm_disable(heartbeat_monitor);
    if (result != hm_Error::NoError)
    {
        std::cout << "Heartbeat monitor disable failed, error: " << static_cast<std::int32_t>(result) << std::endl;
    }

    auto heartbeat_result = hm_hbm_report_heartbeat(heartbeat_monitor);
    if (!heartbeat_result.has_value())
    {
        std::cout << "Heartbeat monitor report heartbeat failed, error: " << static_cast<std::int32_t>(heartbeat_result.error()) << std::endl;
    }
    else if (heartbeat_result.value() == hm_Status::Disabled)
    {
        std::cout << "Heartbeat monitor is disabled." << std::endl;
    }

    result = hm_hbm_enable(heartbeat_monitor);
    if (result != hm_Error::NoError)
    {
        std::cout << "Heartbeat monitor enable failed, error: " << static_cast<std::int32_t>(result) << std::endl;
    }

    heartbeat_result = hm_hbm_report_heartbeat(heartbeat_monitor);
    if (!heartbeat_result.has_value())
    {
        std::cout << "Heartbeat monitor report heartbeat failed, error: " << static_cast<std::int32_t>(heartbeat_result.error()) << std::endl;
    }
    else if (heartbeat_result.value() == hm_Status::Disabled)
    {
        std::cout << "Heartbeat monitor is disabled." << std::endl;
    }
    else
    {
        std::cout << "Heartbeat success" << std::endl;
    }

    hm_hbm_delete(&heartbeat_monitor);
    return 0;
}
