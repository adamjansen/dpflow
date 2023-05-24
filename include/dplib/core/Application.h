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
 * @file application.h
 * @author ajansen
 * @date 2023-04-26
 */

#pragma once
#include <memory>
#include <string>

#include "dplib/core/EventDispatcher.h"
#include "dplib/core/Platform.h"

namespace datapanel
{
namespace core
{

class Event;

/**
 * @brief High-level application
 */
class Application
{
  public:
    static Application &instance()
    {
        static Application _app;
        return _app;
    }

    int run();
    void processEvents();
    void exit(int status = 0);

    int addTimer(int periodMs, EventDispatcher::TimerFunc f)
    {
        return platform->addTimer(periodMs, f);
    }
    bool removeTimer(int id)
    {
        return platform->removeTimer(id);
    }

    bool addFile(int fd, EventDispatcher::FileOperation op, EventDispatcher::FileFunc f)
    {
        return platform->addFile(fd, op, f);
    }
    bool removeFile(int fd, EventDispatcher::FileOperation op)
    {
        return platform->removeFile(fd, op);
    }

    Application(const Application &) = delete;
    void operator=(const Application &) = delete;

  protected:
    bool running;
    int exitStatus = 0;
    bool started;

  private:
    std::shared_ptr<spdlog::logger> m_logger;
    Application();
    ~Application();
    std::unique_ptr<Platform> platform;
};

}  // namespace core
}  // namespace datapanel
