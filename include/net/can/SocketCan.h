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

#pragma once

#include <string>

#include "ICanChannel.h"

namespace datapanel {
  namespace net {
    namespace can {

      class SocketCan {
      public:
        SocketCan();
        SocketCan(const SocketCan &) = delete;
        SocketCan &operator=(const SocketCan &) = delete;
        int open(const std::string &interfaceName, int32_t timeoutMs);
        int send(const CanFrame &frame);
        int recv(CanFrame &frame);
        int close();

        ~SocketCan();

      private:
        int _socket = -1;
        int32_t _timeoutMs;
        std::string _interface;
      };
    }  // namespace can
  }    // namespace net
}  // namespace datapanel
