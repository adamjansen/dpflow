#pragma once
#include <memory>
#include <string>

#include "dplib/event_loop.h"

class Platform;

class Application
{
  public:
    Application(std::string name);
    Application(std::string name, int argc, char **argv);
    ~Application();

    bool create();
    int run();

    virtual void start()
    {
    }
    virtual void stop()
    {
    }

  private:
    std::string name;
    std::unique_ptr<Platform> platform;
};
