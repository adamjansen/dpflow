#include <dplib/net/can/CanFrame.h>
#include <iostream>
#include <sigslot/signal.hpp>
#include <spdlog/spdlog.h>

#include <cxxopts.hpp>
#include <initializer_list>

template<typename... Ts>
std::vector<std::byte> make_payload(Ts&&... args) noexcept {
    return{std::byte(std::forward<Ts>(args))...};
}

auto main(int argc, char **argv) -> int
{
    cxxopts::Options options(*argv, "Sigslot sandbox");

    // clang-format off
  options.add_options()
    ("h,help", "Show help")
    ("v,verbose", "More output", cxxopts::value<bool>()->default_value("false"))
  ;
    // clang-format on
    //

    auto result = options.parse(argc, argv);


    if (result["help"].as<bool>()) {
        std::cout << options.help() << std::endl;
        return 0;
    }

  sigslot::signal<datapanel::net::can::CanFrame&> canFrameReceived;

  auto printer = [](datapanel::net::can::CanFrame &frame) {
	spdlog::info(" {}", frame);
  };

  auto con = canFrameReceived.connect(printer);



auto payload = make_payload(0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x56, 0x78);
  datapanel::net::can::CanFrame f;
  f.setId(0x18EFD027);
  f.setTimestamp(datapanel::net::can::CanFrame::Timestamp::fromNanoseconds(1234567890));
  f.setPayload(payload);
  f.setFrameType(datapanel::net::can::CanFrame::FrameType::DataFrame);
  canFrameReceived(f);


}
