#include <algorithm>
#include <string_view>
#include <cstring>
#include <filesystem>

#include <fmt/format.h>

#include "dplib/net/can/SocketCanBackend.h"

#include "dplib/net/can/CanBus.h"
#include "dplib/net/can/CanInterface.h"

#include <linux/can/error.h>
#include <linux/can/raw.h>
#include <linux/sockios.h>
#include <errno.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <spdlog/spdlog.h>

#include <libsocketcan.h>

using namespace datapanel::net::can;

SocketCanBackend::~SocketCanBackend()
{
    close();
}

std::unique_ptr<CanInterface> SocketCanBackend::init(const std::string &channel)
{
    return std::unique_ptr<SocketCanBackend>(new SocketCanBackend(channel));
}

static int _getMtu(int s, const char *ifname)
{
    struct ifreq ifr;
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    int result = ioctl(s, SIOCGIFMTU, (void *)&ifr);

    if (result != 0) {
        return -1;
    }
    return ifr.ifr_mtu;
}

static std::string _getDriverName(const char *ifname)
{
    std::string module(fmt::format("/sys/class/net/{:s}/device/driver/module", ifname));
    if (std::filesystem::exists(module)) {
        return std::filesystem::read_symlink(module).filename();
    }
    if (std::filesystem::exists(fmt::format("/sys/devices/virtual/net/{:s}", ifname))) {
        return "vcan";
    }
    return "unknown";
}

std::list<CanInterfaceInfo> SocketCanBackend::availableChannels()
{
    std::list<CanInterfaceInfo> channels;

    struct if_nameindex *if_nidxs, *intf;

    if_nidxs = if_nameindex();
    if (if_nidxs != NULL) {
        /* Determine which interfaces are SocketCAN by
         * creating a CAN_RAW socket and trying to
         * bind it to the interface.
         *
         * Non-CAN Interfaces should set errno=ENODEV.
         *
         * A socket can be re-bound before connect()-ing
         * or accept()-ing, so it shouldn't cause any
         * problems to re-use the same socket.
         */
        int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        for (intf = if_nidxs; intf->if_index != 0 || intf->if_name != NULL; intf++) {
            struct sockaddr_can addr;
            addr.can_family = AF_CAN;
            addr.can_ifindex = intf->if_index;
            int status = bind(s, (struct sockaddr *)&addr, sizeof(addr));
            if (status == 0) {
                CanInterfaceInfo info;
                info.plugin = "SocketCAN";
                info.name = intf->if_name;
                info.description = _getDriverName(intf->if_name);
                int mtu = _getMtu(s, intf->if_name);
                if (mtu == CAN_MTU) {
                    info.supportsFD = false;
                } else if (mtu == CANFD_MTU) {
                    info.supportsFD = true;
                } else {
                    // Unrecognized mtu
                    info.supportsFD = false;
                }
                channels.push_back(info);
            }
        }
        if_freenameindex(if_nidxs);
    } else {
        // could not get list of interfaces
    }
    return channels;
}

bool SocketCanBackend::applyConfigOption(CanInterface::ConfigOption opt, const CanInterface::ConfigOptionValue &value)
{
    bool ok = false;

    switch (opt) {
        case ConfigOption::CfgOptBitrate: {
            uint32_t bitrate = static_cast<uint32_t>(std::get<int>(value));
            int status = ::can_set_bitrate(_ifname.c_str(), bitrate);
            if (status < 0) {
                ok = false;
                setError(fmt::format("Could not set bitrate: {}", ::strerror(errno)),
                         CanInterface::CanBusError::ConfigurationError);
            }
            break;
        } break;
        case ConfigOption::CfgOptLoopback: {
            int loopback = std::get<bool>(value) ? 1 : 0;
            int status = ::setsockopt(_socket, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback));
            if (status < 0) {
                ok = false;
                setError(fmt::format("Could not {} loopback: {}", loopback ? "enable" : "disable", ::strerror(errno)),
                         CanInterface::CanBusError::ConfigurationError);
            }
            break;
        }
        case ConfigOption::CfgOptRxOwn: {
            int rxOwn = std::get<bool>(value) ? 1 : 0;
            int status = ::setsockopt(_socket, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &rxOwn, sizeof(rxOwn));
            if (status < 0) {
                ok = false;
                setError(fmt::format("Could not {} receive own messages: {}", rxOwn ? "enable" : "disable",
                                     ::strerror(errno)),
                         CanInterface::CanBusError::ConfigurationError);
            }
        } break;
        case ConfigOption::CfgOptFD: {
            const int fdEnabled = std::get<bool>(value) ? 1 : 0;
            int status = ::setsockopt(_socket, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &fdEnabled, sizeof(fdEnabled));
            if (status < 0) {
                ok = false;
                setError(
                    fmt::format("Could not {} CAN fd frames: {}", fdEnabled ? "enable" : "disable", ::strerror(errno)),
                    CanInterface::CanBusError::ConfigurationError);
            }
        } break;

        default:
            setError(fmt::format("Unsupported configuration option {}", opt),
                     CanInterface::CanBusError::ConfigurationError);
            break;
    }
    return ok;
}

void SocketCanBackend::setConfigOption(ConfigOption opt, const ConfigOptionValue& value)
{
    if (_socket != -1 && !applyConfigOption(opt, value))
        return;

    CanInterface::setConfigOption(opt, value);

    if (opt == CfgOptFD)
        _fdEnabled = std::get<bool>(value);
}

bool SocketCanBackend::send(const CanFrame &frame)
{
    if (state() != ConnectedState) {
        return false;
    }

    if (!frame.isValid()) {
        setError("Cannot write invalid frame", CanInterface::CanBusError::TxError);
        return false;
    }

    canid_t id = frame.id();
    if (frame.isExtendedId())
        id |= CAN_EFF_FLAG;

    if (frame.frameType() == CanFrame::RemoteRequestFrame) {
        id |= CAN_RTR_FLAG;
    } else if (frame.frameType() == CanFrame::ErrorFrame) {
        id = static_cast<canid_t>((frame.error()) & CanFrame::AnyError);
        id |= CAN_ERR_FLAG;
    }

    if (frame.isFD() && !_fdEnabled) {
        setError("Cannot send FD frame when FD is disabled", CanInterface::TxError);
        return false;
    }

    int written = 0;
    if (frame.isFD()) {
        canfd_frame tx = {};
        tx.len = frame.payload().size();
        tx.can_id = id;
        tx.flags = frame.isBitrateSwitch() ? CANFD_BRS : 0;
        tx.flags |= frame.isErrorState() ? CANFD_ESI : 0;
        ::memcpy(tx.data, frame.payload().data(), tx.len);
        written = ::write(_socket, &tx, sizeof(tx));
    } else {
        can_frame tx = {};
        tx.can_dlc = frame.payload().size();
        tx.can_id = id;
        ::memcpy(tx.data, frame.payload().data(), tx.can_dlc);
        written = ::write(_socket, &tx, sizeof(tx));
    }

    if (written < 0) {
        setError(fmt::format("Could not send frame: {}", ::strerror(errno)), CanInterface::TxError);
        return false;
    }

    return true;
}

bool SocketCanBackend::restart()
{
    return ::can_do_restart(_ifname.c_str()) == 0;
}

CanInterface::CanBusState SocketCanBackend::busStatus()
{
    int status;
    int result = ::can_get_state(_ifname.c_str(), &status);
    if (result < 0)
        return CanInterface::CanBusState::Unknown;

    switch (status) {
        case CAN_STATE_ERROR_PASSIVE:
            return CanInterface::CanBusState::Error;
        case CAN_STATE_ERROR_ACTIVE:
            return CanInterface::CanBusState::OK;
        case CAN_STATE_ERROR_WARNING:
            return CanInterface::CanBusState::Warning;
        case CAN_STATE_BUS_OFF:
            return CanInterface::CanBusState::BusOff;
        default:
            return CanInterface::CanBusState::Unknown;
    }
}

bool SocketCanBackend::open()
{
    if (_socket != -1) {
        return false;  // already opened
    }

    struct ifreq interface;
    if ((_socket = ::socket(PF_CAN, SOCK_RAW | SOCK_NONBLOCK, CAN_RAW)) < 0) {
        setError(fmt::format("Could not open socket: {}", ::strerror(errno)),
                 CanInterface::CanBusError::ConnectionError);
        return false;
    }
    spdlog::debug("Opened socket {:d}", _socket);
    ::strncpy(interface.ifr_name, _ifname.c_str(), sizeof(interface.ifr_name));
    if (::ioctl(_socket, SIOCGIFINDEX, &interface) < 0) {
        setError(fmt::format("Could not get interface index: {}", ::strerror(errno)),
                 CanInterface::CanBusError::ConnectionError);
        return false;
    }

    _addr.can_family = AF_CAN;
    _addr.can_ifindex = interface.ifr_ifindex;

    if (::bind(_socket, reinterpret_cast<struct sockaddr *>(&_addr), sizeof(_addr)) < 0) {
        setError(fmt::format("Could not bind to interface {}: {}", _ifname, ::strerror(errno)),
                 CanInterface::CanBusError::ConnectionError);
        return false;
    }

    _iov.iov_base = &_raw_frame;
    _msg.msg_name = &_addr;
    _msg.msg_iov = &_iov;
    _msg.msg_iovlen = 1;
    _msg.msg_control = &_ctrlmsg;

    setState(CanInterface::ConnectedState);

    const auto opts = configOptions();
    for (ConfigOption opt : opts) {
        const ConfigOptionValue value = configOption(opt);
        bool ok = applyConfigOption(opt, value);
        if (!ok) {
            spdlog::error("Cannot apply option {}={}", opt, value);
        }
    }

    _eventLoop->enqueue([&]
    {
        struct epoll_event event;
        int epoll_fd = ::epoll_create1(0);
        if (epoll_fd == -1)
        {
            spdlog::error("epoll failed: {}", ::strerror(errno));
            return;
        }

        event.events = EPOLLIN;
        event.data.fd = _socket;
        if (::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _socket, &event)) {
            spdlog::error("epoll_ctl failed: {}", ::strerror(errno));
            ::close(epoll_fd);
            return;
        }

        while (1) {
            struct epoll_event rx_event;
            int count = ::epoll_wait(epoll_fd, &rx_event, 1, 100);
            if (count > 0) {
                readSocket();
            }
            if (_socket == -1) {
                // socket was closed while we were waiting
                break;
            }
        }

        if (::close(epoll_fd)) {
            spdlog::error("epoll close failed: {}", strerror(errno));
        }
    });

    return true;
}

bool SocketCanBackend::close()
{
    ::close(_socket);
    _socket = -1;
    setState(CanInterface::DisconnectedState);
    return false;
}


void SocketCanBackend::readSocket()
{
    std::list<CanFrame> frames;

    while (true) {
        _raw_frame = {};
        _iov.iov_len = sizeof(_raw_frame);
        _msg.msg_namelen = sizeof(_addr);
        _msg.msg_controllen = sizeof(_ctrlmsg);
        _msg.msg_flags = 0;

        const int bytesRx = ::recvmsg(_socket, &_msg, 0);

        if (bytesRx <= 0) {
            break;
        } else if (bytesRx != CANFD_MTU && bytesRx != CAN_MTU) {
            spdlog::error("Incomplete CAN frame");
            setError("Incomplete CAN frame", CanInterface::CanBusError::RxError);
            continue;
        } else if (_raw_frame.len > bytesRx - offsetof(canfd_frame, data)) {
            setError("Invalid CAN frame length", CanInterface::CanBusError::RxError);
            spdlog::error("Invalid CAN frame length");
            continue;
        }

        struct timeval ts = {};
        if (::ioctl(_socket, SIOCGSTAMP, &ts) < 0) {
            setError(fmt::format("RX error: {}", ::strerror(errno)), CanInterface::CanBusError::RxError);
            spdlog::error("That timestamp was whack");
            ts = {};
        }

        const CanFrame::Timestamp timestamp(ts.tv_sec, 1000*ts.tv_usec);
        CanFrame frame;
        frame.setTimestamp(timestamp);
        frame.setFD(bytesRx == CANFD_MTU);
        frame.setExtendedId(_raw_frame.can_id & CAN_EFF_FLAG);

        if (_raw_frame.can_id & CAN_RTR_FLAG)
            frame.setFrameType(CanFrame::RemoteRequestFrame);
        else if (_raw_frame.can_id & CAN_ERR_FLAG)
            frame.setFrameType(CanFrame::ErrorFrame);
        else
            frame.setFrameType(CanFrame::DataFrame);
        if (_raw_frame.flags & CANFD_BRS)
            frame.setBitrateSwitch(true);
        if (_raw_frame.flags & CANFD_ESI)
            frame.setErrorState(true);
        if (_msg.msg_flags & MSG_CONFIRM)
            frame.setLocalEcho(true);

        frame.setId(_raw_frame.can_id & CAN_EFF_MASK);

        std::basic_string_view<uint8_t> sview(_raw_frame.data, _raw_frame.len);
        std::vector<std::byte> data;
        std::transform(sview.cbegin(), sview.cend(), std::back_inserter(data), [](unsigned char c) { return std::byte(c); });
        frame.setPayload(data);

        frames.push_back(std::move(frame));
    }

    enqueueRxFrames(frames);
}
