#include <spdlog/sinks/stdout_color_sinks.h>

#include "dplib/net/can/CanBus.h"
#include "dplib/net/can/SocketCanBackend.h"

#include "dplib/platform.h" 

using namespace datapanel::net::can;

class PlatformLinux final : public Platform
{
    public:
        PlatformLinux(Application& app);

        bool appStart(int argc, char **argv) override;
        void appStop() override;


        Application& app;

private:
    std::shared_ptr<spdlog::logger> logger;
};


std::unique_ptr<Platform> createPlatform(Application& app)
{
    return std::make_unique<PlatformLinux>(app);
}

PlatformLinux::PlatformLinux(Application& app)
    : app(app)
{
    logger = spdlog::stdout_color_st("PlatformLinux");
    m_eventLoop = EventLoop::getDefault();
}

bool PlatformLinux::appStart(int argc, char **argv)
{
    logger->info("Starting platform");

    CanBusPluginInfo info{
        "SocketCAN",
        SocketCanBackend::init,
        SocketCanBackend::availableChannels
    };
    CanBus::registerPlugin(info);

    logger->info("Registered SocketCAN plugins");
    return true;
}

void PlatformLinux::appStop()
{
    logger->info("Stopping platform");
    m_eventLoop->stop();
}
