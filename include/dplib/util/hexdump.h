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
 * @file hexdump.h
 * @author ajansen
 * @date 2023-04-26
 */

#pragma once

#include <string>
#include <vector>

namespace datapanel
{
namespace util
{

/**
 * @brief Format raw binary data as hexadecimal for inspection
 *
 * Each input byte of @p data will be formatted as uppercase
 * hexadecimal and separated by @p sep.  The resulting
 * string is returned.
 *
 * @param[in] data Raw data to format
 * @param[in] sep Will be inserted between each byte of data
 *
 * @return @p data formatted in hex
 *
 * @since 1.0
 */
std::string hexdump(const std::vector<std::byte> &data, const std::string &sep = " ");
}  // namespace util
}  // namespace datapanel
