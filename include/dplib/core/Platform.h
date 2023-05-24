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
 * @file platform.h
 * @author ajansen
 * @date 2023-04-26
 */

#pragma once

#include <memory>

namespace datapanel
{
namespace core
{

class Application;

/**
 * @brief Represent a consistent interface for applications
 *
 * This class hides platform-specific details from applications,
 * making cross-platform support easier.
 */
class Platform
{
  public:
    virtual ~Platform(){};

    /**
     * @brief Subclasses should override this for initialization
     *
     * @return true on success, or false if initialization failed
     */
    virtual bool appStart() = 0;

    /**
     * @brief Subclasses should override this for shutdown
     */
    virtual void appStop() = 0;

    virtual void processEvents() = 0;

    int addTimer(int periodMs, EventDispatcher::TimerFunc f)
    {
        return m_eventDispatcher.addTimer(periodMs, f);
    }
    bool removeTimer(int id)
    {
        return m_eventDispatcher.removeTimer(id);
    }

    bool addFile(int fd, EventDispatcher::FileOperation op, EventDispatcher::FileFunc f)
    {
        return m_eventDispatcher.addFile(fd, op, f);
    }
    bool removeFile(int fd, EventDispatcher::FileOperation op)
    {
        return m_eventDispatcher.removeFile(fd, op);
    }

  protected:
    EventDispatcher m_eventDispatcher;
};
std::unique_ptr<Platform> createPlatform();

}  // namespace core
}  // namespace datapanel
