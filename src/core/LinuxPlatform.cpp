#include <spdlog/sinks/stdout_color_sinks.h>

#include "dplib/core/EventDispatcher.h"
#include "dplib/net/can/CanBus.h"
#include "dplib/net/can/SocketCanBackend.h"

#include "dplib/core/Platform.h"

#include <termios.h>

using namespace datapanel::core;
using namespace datapanel::net::can;

class PlatformLinux final : public Platform
{
  public:
    PlatformLinux();
    ~PlatformLinux();

    bool appStart() override;
    void appStop() override;
    void processEvents() override;

  private:
    std::shared_ptr<spdlog::logger> m_logger;
};

std::unique_ptr<Platform> datapanel::core::createPlatform()
{
    return std::make_unique<PlatformLinux>();
}

PlatformLinux::PlatformLinux() : Platform()
{
    m_logger = spdlog::stdout_color_st("PlatformLinux");
}

PlatformLinux::~PlatformLinux()
{
    m_logger->flush();
    m_logger.reset();
}

bool PlatformLinux::appStart()
{
    m_logger->info("Starting platform");

    CanBusPluginInfo info{"SocketCAN", SocketCanBackend::init, SocketCanBackend::availableChannels};
    CanBus::registerPlugin(info);

    m_logger->info("Registered SocketCAN plugins");

    struct termios ctrl;
    tcgetattr(STDIN_FILENO, &ctrl);
    ctrl.c_lflag &= ~ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &ctrl);

    return true;
}

void PlatformLinux::processEvents()
{
    m_eventDispatcher.processEvents();
}

void PlatformLinux::appStop()
{
    m_logger->info("Stopping platform {:p}", (void *)this);
}
