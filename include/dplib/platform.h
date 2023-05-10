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

#include "dplib/event_loop.h"

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
    virtual bool appStart(int argc, char **argv) = 0;

    /**
     * @brief Subclasses should override this for shutdown
     */
    virtual void appStop() = 0;

    /**
     * @brief Get the default Event Loop for the platform
     */
    std::shared_ptr<EventLoop> getEventLoop() const
    {
        return m_eventLoop;
    }

  protected:
    std::shared_ptr<EventLoop> m_eventLoop = nullptr;
};

/**
 * @brief Construct a platform for the given application
 *
 * @param[in] app Running application
 */
std::unique_ptr<Platform> createPlatform(Application &app);
