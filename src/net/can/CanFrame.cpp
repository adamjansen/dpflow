/**
 * Copyright (c) 2023 Data Panel Corporation
 * 181 Cheshire Ln, Suite 300
 * Plymouth, MN 55441
 * All rights reserved.
 *
 * This is the confidential and proprietary information of Data Panel
 * Corporation. Such confidential information shall not be disclosed and is for
 * use only in accordance with the license agreement you entered into with Data
 * Panel.
 */

/**
 * @file
 * @author ajansen
 * @date 2023-04-27
 */

#include <string>
#include <iterator>
#include <chrono>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <fmt/core.h>
#include <fmt/chrono.h>

#include "dplib/net/can/CanFrame.h"

using namespace datapanel::net::can;
