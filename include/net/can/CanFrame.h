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
 * @file
 * @author ajansen
 * @date 2023-04-26
 */

#pragma once

#include <fmt/chrono.h>
#include <fmt/core.h>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

namespace datapanel {
  namespace net {
    namespace can {
      constexpr uint32_t CAN_EFF_FLAG = 0x80000000U;
      constexpr uint32_t CAN_RTR_FLAG = 0x40000000U;
      constexpr uint32_t CAN_ERR_FLAG = 0x40000000U;

      constexpr uint32_t CAN_STD_ID_BITS = 11;
      constexpr uint32_t CAN_EXT_ID_BITS = 29;

      constexpr uint32_t CAN_STD_MASK = 0x000007FFU;
      constexpr uint32_t CAN_EXT_MASK = 0x1FFFFFFFU;
      constexpr uint32_t CAN_ERR_MASK = CAN_EXT_MASK;

      struct CanId {
      public:
        CanId(const CanId& orig) : _identifier(orig._identifier) { /* copy */
        }

        CanId(const uint32_t identifier) : _identifier(identifier) {
          if (!isValid(identifier)) {
            throw std::invalid_argument("Invalid 11-bit identifier");
          }
          _identifier = identifier & CAN_STD_MASK;
        }

        operator uint16_t() const {
          return isStandardFrameId()
                     ? static_cast<uint16_t>(_identifier)
                     : throw std::invalid_argument("Cannot convert extended id to int16_t");
        }
        operator uint32_t() const { return _identifier; }

        CanId operator&(CanId& x) const { return _identifier & x._identifier; }
        CanId operator&(const CanId x) const { return _identifier & x._identifier; }
        CanId operator&(const uint16_t x) const { return _identifier & x; }
        CanId operator&(const uint32_t x) const { return _identifier & x; }
        CanId operator&(const uint64_t x) const { return _identifier & x; }

        CanId operator|(CanId& x) const { return _identifier | x._identifier; }
        CanId operator|(const CanId x) const { return _identifier | x._identifier; }
        CanId operator|(const uint16_t x) const { return _identifier | x; }
        CanId operator|(const uint32_t x) const { return _identifier | x; }
        CanId operator|(const uint64_t x) const { return _identifier | x; }

        // TODO: how should ID comparisons between rtr and non-rtr occur?
        bool operator==(CanId& x) const { return _identifier == x._identifier; }
        bool operator==(const CanId& x) const { return _identifier == x._identifier; }
        bool operator==(const uint16_t x) const { return _identifier == x; }
        bool operator==(const uint32_t x) const { return _identifier == x; }

        bool operator!=(CanId& x) const { return _identifier != x._identifier; }
        bool operator!=(const CanId& x) const { return _identifier != x._identifier; }
        bool operator!=(const uint16_t x) const { return _identifier != x; }
        bool operator!=(const uint32_t x) const { return _identifier != x; }

        bool operator<(CanId& x) const { return x._identifier < _identifier; }
        bool operator<(const CanId& x) const { return x._identifier < _identifier; }
        bool operator<(const uint16_t x) const { return x < _identifier; }
        bool operator<(const uint32_t x) const { return x < _identifier; }
        bool operator<=(CanId& x) const { return x._identifier <= _identifier; }
        bool operator<=(const CanId& x) const { return x._identifier <= _identifier; }

        bool operator>(CanId& x) const { return x > _identifier; }
        bool operator>(const CanId& x) const { return x > _identifier; }
        bool operator>=(CanId& x) const { return x._identifier >= _identifier; }
        bool operator>=(const CanId& x) const { return x._identifier >= _identifier; }
        bool operator>(const uint16_t x) const { return x > _identifier; }
        bool operator>(const uint32_t x) const { return x > _identifier; }

        CanId& operator=(uint32_t newid) {
          _identifier = newid;
          return *this;
        }
        CanId& operator=(const CanId& x) {
          _identifier = x._identifier;
          return *this;
        }

        CanId operator+(CanId& x) const { return _identifier + x._identifier; }
        CanId operator+(const CanId& x) const { return _identifier + x._identifier; }
        CanId operator+(const uint16_t x) const { return _identifier + x; }
        CanId operator+(const uint32_t x) const { return _identifier + x; }

        CanId operator-(CanId& x) const { return _identifier - x._identifier; }
        CanId operator-(const CanId& x) const { return _identifier - x._identifier; }
        CanId operator-(const int16_t x) const { return _identifier - x; }
        CanId operator-(const uint16_t x) const { return _identifier - x; }
        CanId operator-(const int32_t x) const { return _identifier - x; }
        CanId operator-(const uint32_t x) const { return _identifier - x; }

        operator std::string() const {
          auto precision = isExtendedFrameId() ? 8 : 3;
          return fmt::format("0x{:0{}X}", _identifier & CAN_EXT_MASK, precision);
        }

      public:
        bool isExtendedFrameId() const { return _identifier & CAN_EFF_FLAG; }
        bool isStandardFrameId() const { return !(_identifier & CAN_EFF_FLAG); }
        bool isRtr() const { return _identifier & CAN_RTR_FLAG; }
        bool isError() const { return _identifier & CAN_ERR_FLAG; }
        bool equals(CanId other) const { return *this == other; }

        bool isValid(uint32_t identifier) {
          if (!(identifier & CAN_EFF_FLAG) && (identifier > CAN_EXT_MASK)) {
            return false;
          }
          return true;
        }
        bool isValid() { return isValid(_identifier); }

      private:
        uint32_t _identifier = 0;
      };

      class CanFrame {
      public:
        CanFrame() : _identifier(0), _data("") {}
        CanFrame(const CanId id, const std::string data) : _identifier(id), _data(data) {
          if (data.size() > 8) throw std::invalid_argument("Payload too large for frame");
          _timestamp = std::chrono::system_clock::now();
        }

        virtual ~CanFrame() {}

        const CanId getId() const { return this->_identifier; }
        const std::string getData() const { return this->_data; }
        const std::chrono::time_point<std::chrono::system_clock> getTimestamp() const {
          return this->_timestamp;
        }

        void setId(CanId id) { _identifier = id; }
        void setData(std::string data) { _data = data; }
        void setTimestamp(std::chrono::time_point<std::chrono::system_clock> timestamp) {
          _timestamp = timestamp;
        }

        operator std::string() const {
          auto ns
              = std::chrono::duration_cast<std::chrono::microseconds>(_timestamp.time_since_epoch())
                    .count()
                % 1000000;
          return fmt::format(
              "{:%Y-%m-%d-%H:%M:%S}.{:06d} {:s} [{:d}] {:s} {:s} {:s} {:s} {:s} {:s} {:s} {:s}",
              _timestamp, ns, std::string(_identifier), _data.size(),
              (_data.size() > 0) ? fmt::format("{:02X}", _data[0]) : "",
              (_data.size() > 1) ? fmt::format("{:02X}", _data[1]) : "",
              (_data.size() > 2) ? fmt::format("{:02X}", _data[2]) : "",
              (_data.size() > 3) ? fmt::format("{:02X}", _data[3]) : "",
              (_data.size() > 4) ? fmt::format("{:02X}", _data[4]) : "",
              (_data.size() > 5) ? fmt::format("{:02X}", _data[5]) : "",
              (_data.size() > 6) ? fmt::format("{:02X}", _data[6]) : "",
              (_data.size() > 7) ? fmt::format("{:02X}", _data[7]) : "");
        }

      private:
        CanId _identifier;
        std::string _data;
        std::chrono::time_point<std::chrono::system_clock> _timestamp;
      };

    }  // namespace can
  }    // namespace net

}  // namespace datapanel
