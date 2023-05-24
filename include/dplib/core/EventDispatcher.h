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
 * @file EventDispatcher.h
 * @author ajansen
 * @date 2023-05-10
 */

#pragma once

#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <chrono>

namespace datapanel
{
namespace core
{

class Timer;
class EventDispatcher
{
  public:
    enum FileOperation {Read, Write, Error};

      using TimerFunc = std::function<void()>;
      using FileFunc = std::function<void()>;

    explicit EventDispatcher();
    ~EventDispatcher();

    virtual bool processEvents();
    virtual bool pendingEvents();

    int addTimer(int periodMs, TimerFunc f);
    bool removeTimer(int id);

    bool addFile(int fd, FileOperation op, FileFunc f);
    bool removeFile(int fd, FileOperation op);

  protected:
    bool m_interrupt;

    struct TimerInfo {
        int id;
        std::chrono::duration<int, std::chrono::milliseconds::period> period;
        std::chrono::time_point<std::chrono::steady_clock> expiry;
        std::function<void()> func;
        TimerInfo(int id, int periodMs, TimerFunc func)
        : id(id), func(func) {
            period = std::chrono::milliseconds(periodMs);
            expiry = std::chrono::steady_clock::now() + period;
        }
        bool operator<(const TimerInfo& rhs) const { return expiry < rhs.expiry;}
        bool expired() const {
            auto now = std::chrono::steady_clock::now();
            return now >= expiry;
        }
        void fire() {
            func();
            expiry = std::chrono::steady_clock::now() + period;
        }
    };

    std::list<TimerInfo> m_timers;
    std::map<std::pair<int,FileOperation>, FileFunc> m_files;
    std::mutex m_mutex;
    int m_epoll_fd;;
};

}  // namespace core
}  // namespace datapanel
