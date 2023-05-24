#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>

#include <dplib/version.h>
#include "dplib/net/can/CanBus.h"
#include "dplib/core/Application.h"

#include <cxxopts.hpp>

using namespace datapanel::net::can;

using namespace std::chrono_literals;

auto main(int argc, char **argv) -> int
{
    cxxopts::Options options(*argv, "DPFlow");

    std::string interface;

    // clang-format off
  options.add_options()
    ("h,help", "Show help")
    ("V,version", "Print the current version number")
    ("v,verbose", "More output", cxxopts::value<bool>()->default_value("false"))
    ("i,interface", "CAN interface to use", cxxopts::value(interface)->default_value("SocketCAN.can0"))
    ("s,scan", "Scan for available interfaces", cxxopts::value<bool>()->default_value("false"))
  ;
    // clang-format on
    //

    auto result = options.parse(argc, argv);

    if (result["help"].as<bool>()) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (result["version"].as<bool>()) {
        std::cout << "DPFlow, version " << DPLIB_VERSION << std::endl;
        return 0;
    }

    datapanel::core::Application &app = datapanel::core::Application::instance();

    if (result["scan"].as<bool>()) {
        for (auto &channel : CanBus::availableChannels())
            std::cout << "Channel: " << channel.plugin << "." << channel.name << "\n";
        return 0;
    }

    constexpr int KILL_DELAY_MS = 2000;
    app.addTimer(KILL_DELAY_MS, [&]() { app.exit(0); });

    std::string plugin = interface.substr(0, interface.find("."));
    std::string channel = interface.substr(plugin.length() + 1, interface.length());
    spdlog::info("Connecting to {}.{}", plugin, channel);

    auto bus = CanBus::create(plugin, channel);

    bus->errorOccurred.connect([](CanInterface::CanBusError error) { spdlog::error("Connection error: {}", error); });
    bus->connectionStateChanged.connect(
        [](CanInterface::CanConnectionState state) { spdlog::info("Connection state changed to {}", state); });
    bus->framesReceived.connect([&]() {
        CanFrame frame = bus->recv();
        while (frame.frameType() != CanFrame::InvalidFrame) {
            spdlog::info("RX: {}", frame);
            frame = bus->recv();
        }
    });

    bus->connect();

    return app.run();
}
