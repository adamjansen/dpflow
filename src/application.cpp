#include "dplib/application.h"
#include "dplib/platform.h"


Application::Application(std::string name)
    : Application(name, 0, nullptr)
{
}

Application::Application(std::string name, int argc, char **argv)
: name(name),
  platform(createPlatform(*this))
{
    platform->appStart(argc, argv);
}

Application::~Application()
{
    platform->appStop();
}

bool Application::create()
{
    start();
    return true;
}

int Application::run() {

    stop();
    return 0;
}
