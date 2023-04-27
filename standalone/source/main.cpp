#include <chrono>
#include <loguru.hpp>

#include <dpflow/dpflow.h>
#include <dpflow/version.h>

#include "net/can/SocketCan.h"

#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <unordered_map>

#include <sol/sol.hpp>

auto main(int argc, char** argv) -> int {

  cxxopts::Options options(*argv, "A program to welcome the world!");

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

  //loguru::init(argc, argv);

  auto result = options.parse(argc, argv);

  if (result["help"].as<bool>()) {
    std::cout << options.help() << std::endl;
    return 0;
  }

  if (result["version"].as<bool>()) {
    std::cout << "DPFlow, version " << DPFLOW_VERSION << std::endl;
    return 0;
  }

  if (result["verbose"].as<bool>()) {
    loguru::g_stderr_verbosity = 1;
  } else {
    loguru::g_stderr_verbosity = 0;
  }

  datapanel::net::can::SocketCan can0;
  int status = can0.open(interface.c_str(), 1000);
  LOG_F(INFO, "open returned %d", status);

  datapanel::net::can::CanFrame frame;
  status = can0.recv(frame);
  LOG_F(INFO, "recv returned %d", status);

  std::cout << std::string(frame) << std::endl;


  sol::state lua;
  int x = 0;
  lua.set_function("beep", [&x]{++x;});
  lua.script("beep()");

  std::cout << "X: " << x << std::endl;


  return 0;
}
