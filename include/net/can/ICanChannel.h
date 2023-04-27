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

#include <cstdint>
#include <stdexcept>
#include <string>

#include "CanFrame.h"

namespace datapanel {
  namespace net {
    namespace can {

      class ICanChannel {
      public:
        virtual ~ICanChannel(){};
        virtual bool send(CanFrame& frame) = 0;
        virtual bool recv(CanFrame& frame) = 0;

      protected:
        ICanChannel(){};
      };
    }  // namespace can
  }    // namespace net
}  // namespace datapanel
