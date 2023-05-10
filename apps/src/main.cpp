#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>

#include <dplib/version.h>
#include "dplib/net/can/CanBus.h"
#include "dplib/application.h"

#include "dplib/event_loop.h"

#include <cxxopts.hpp>

using namespace datapanel::net::can;

using namespace std::chrono_literals;

class DPFlow : public Application
{
  using Application::Application;
public:
    virtual void start() override {

      for (auto &channel : CanBus::availableChannels())
        std::cout << "Channel: " << channel.plugin << "." << channel.name << "\n";

      auto b = CanBus::create("SocketCAN", "can0");
      b->connect();

      //std::this_thread::sleep_for(500ms);

      CanFrame frame = b->recv();
      std::cout << fmt::format("Frame: {}", frame) << std::endl;
      b->disconnect();
    }
    virtual void stop() override {}
};


auto main(int argc, char **argv) -> int
{
    cxxopts::Options options(*argv, "DPFlow");

    std::string interface;

    // clang-format off
  options.add_options()
    ("h,help", "Show help")
    ("V,version", "Print the current version number")
    ("v,verbose", "More output", cxxopts::value<bool>()->default_value("false"))
    ("i,interface", "CAN interface to use", cxxopts::value(interface)->default_value("can0"))
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

  DPFlow dpflow(std::string("DPFlow"), argc, argv);
  dpflow.create();
  dpflow.run();
}
