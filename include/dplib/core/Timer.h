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
 * @file Timer.h
 * @author ajansen
 * @date 2023-05-10
 */

#pragma once

namespace datapanel
{
namespace core
{
class Timer
{
  public:
    explicit Timer();
    ~Timer();

    bool active() const
    {
        return m_id >= 0;
    }
    int id() const
    {
        return m_id;
    }

    void setPeriod(int ms);
    int period() const
    {
        return m_period;
    }

    inline void setOneshot(bool oneshot);
    inline bool oneshot() const
    {
        return m_oneshot;
    }

    void start(int periodMs);
    void start();
    void stop();

  private:
    int m_id = 0;
    int m_period = 0;
    bool m_oneshot = false;
};
}  // namespace core
}  // namespace datapanel
