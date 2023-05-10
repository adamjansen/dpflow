#pragma once

#include <memory>

#include "dplib/event_loop.h"

class Application;

class Platform
{
  public:
    virtual ~Platform(){};

    virtual bool appStart(int argc, char **argv) = 0;
    virtual void appStop() = 0;

    std::shared_ptr<EventLoop> getEventLoop() const { return m_eventLoop;}

protected:
  std::shared_ptr<EventLoop> m_eventLoop = nullptr;
};

std::unique_ptr<Platform> createPlatform(Application &app);
