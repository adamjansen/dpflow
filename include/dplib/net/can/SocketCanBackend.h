#pragma once

#include "dplib/net/can/CanFrame.h"
#include "dplib/net/can/CanInterface.h"

#include "dplib/event_loop.h"

#include <spdlog/spdlog.h>

#include <sys/socket.h>
#include <sys/uio.h>
#include <linux/can.h>
#include <sys/time.h>

namespace datapanel
{
namespace net
{
namespace can
{
class SocketCanBackend : public CanInterface
{
  public:
    ~SocketCanBackend();

    bool open() override;
    bool close() override;

    void setConfigOption(ConfigOption opt, const ConfigOptionValue &value) override;
    bool send(const CanFrame &frame) override;

    bool restart() override;
    CanBusState busStatus() override;

    static std::unique_ptr<CanInterface> init(const std::string &channel);

    static void setup();

    static std::list<CanInterfaceInfo> availableChannels();
  private:
    bool applyConfigOption(ConfigOption opt, const ConfigOptionValue& value);

    SocketCanBackend(const std::string &channel) : _ifname(channel){
      _eventLoop = EventLoop::getDefault();
    }

    std::string _ifname;
    int _socket = -1;

    struct sockaddr_can _addr;
    canfd_frame _raw_frame;
    msghdr _msg;
    iovec _iov;
    char _ctrlmsg[CMSG_SPACE(sizeof(timeval)) + CMSG_SPACE(sizeof(uint32_t))];

    void readSocket();

    bool _fdEnabled = false;

    std::shared_ptr<EventLoop> _eventLoop = nullptr;


};

}  // namespace can
}  // namespace net
}  // namespace datapanel
