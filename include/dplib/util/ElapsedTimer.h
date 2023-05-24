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
 * @file ElapsedTimer.h
 * @author ajansen
 * @date 2023-05-10
 */

#pragma once

#include <chrono>

namespace datapanel
{
namespace util
{
class ElapsedTimer
{
  public:
    void start()
    {
        m_start = std::chrono::steady_clock::now();
    }

    std::chrono::duration<double, std::chrono::nanoseconds::period> restart()
    {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::chrono::nanoseconds::period> elapsed{now - m_start};
        m_start = now;
        return elapsed;
    }

    std::chrono::duration<double, std::chrono::nanoseconds::period> elapsed()
    {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::chrono::nanoseconds::period> elapsed{now - m_start};
        return elapsed;
    }

  private:
    std::chrono::time_point<std::chrono::steady_clock> m_start;
};

}  // namespace util
}  // namespace datapanel
