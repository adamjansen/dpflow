/**
 * Copyright (c) 2023 Data Panel Corporation
 * 181 Cheshire Ln, Suite 300
 * Plymouth, MN 55441
 * All rights reserved.
 *
 * This is the confidential and proprietary information of Data Panel
 * Corporation. Such confidential information shall not be disclosed and is for
 * use only in accordance with the license agreement you entered into with Data
 * Panel.
 */

/**
 * @author ajansen
 * @date 2023-04-26
 */


#include <cstdint>
#include <cstring>
#include <string>

#include "net/can/SocketCan.h"

#include <linux/can.h>
#include <fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/net_tstamp.h>
#include <linux/can/raw.h>
#include <unistd.h>

#define LOGURU_WITH_STREAMS 1
#include <loguru.hpp>


namespace datapanel {
  namespace net {
    namespace can {

SocketCan::SocketCan()
{
}

SocketCan::~SocketCan()
{
    close();
}

int SocketCan::open(const std::string& interfaceName, int32_t timeoutMs) {
    _interface = interfaceName;
    _timeoutMs = timeoutMs;
    if ((_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        LOG_F(ERROR, "Could not open socket: %d", _socket);
    }
    LOG_F(1, "Opened socket %d", _socket);
    struct sockaddr_can addr;

    struct ifreq ifr;
    strcpy(ifr.ifr_name, interfaceName.c_str());
    ifr.ifr_ifindex = if_nametoindex(ifr.ifr_name);
    if (!ifr.ifr_ifindex) {
        LOG_F(ERROR, "Could not find interface %s", interfaceName.c_str());
    }
    LOG_F(1, "Using interface %s", interfaceName.c_str());
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    const int timestampFlags = SOF_TIMESTAMPING_SOFTWARE | SOF_TIMESTAMPING_RX_SOFTWARE | SOF_TIMESTAMPING_RAW_HARDWARE;
    if (setsockopt(_socket, SOL_SOCKET, SO_TIMESTAMPING, &timestampFlags, sizeof(timestampFlags)) < 0) {
        LOG_F(INFO, "Timestamping not supported");
    } else {
        LOG_F(1, "Timestamps enabled");
    }
    struct timeval tv;
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;

    setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
    if (bind(_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        LOG_F(ERROR, "Could not set timeout");
    }

    return 0;
}

int SocketCan::close() {
    if (_socket != -1) {
        LOG_F(1, "Closing socket %d", _socket);
        ::close(_socket);
    }
    return 0;
}

int SocketCan::send(const CanFrame& frame) {

    struct can_frame raw;
    memset(&raw, 0, CAN_MTU);
    raw.can_id = frame.getId();
    std::string data = frame.getData();
    raw.len = data.size();
    memcpy(raw.data, data.c_str(), data.size());

    if (::write(_socket, &raw, CAN_MTU) != CAN_MTU) {
        LOG_F(ERROR, "Could not send frame");
    }

    return 0;
}

int SocketCan::recv(CanFrame& frame) {
    struct can_frame raw;
    auto nBytes = ::read(_socket, &raw, CAN_MTU);
    if (nBytes != CAN_MTU) {
        LOG_F(ERROR, "Could not receive frame");
    }

    CanId id(raw.can_id);
    frame.setId(id);
    frame.setData(std::string((const char *)raw.data, raw.len));
    frame.setTimestamp(std::chrono::system_clock::now());
    return 0;
}
    }  // namespace can
  }    // namespace net
}  // namespace datapanel
