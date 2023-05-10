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

#include "dplib/event_loop.h"

class Platform;

/**
 * @brief High-level application
 */
class Application
{
  public:
    Application(std::string name);
    Application(std::string name, int argc, char **argv);
    ~Application();

    /**
     * @brief initialize application
     *
     * @return true on success
     */
    bool create();

    /**
     * @brief Run the application
     *
     * @return exit status
     */
    int run();

    /**
     * @brief Extension point for subclasses
     */
    virtual void start()
    {
    }

    /**
     * @brief Extension point for subclasses
     */
    virtual void stop()
    {
    }

  private:
    std::string name;
    std::unique_ptr<Platform> platform;
};
