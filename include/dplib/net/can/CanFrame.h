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
#include <cstddef>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <string>
#include <bits/types/struct_FILE.h>

#include "dplib/util/hexdump.h"

namespace datapanel
{
namespace net
{
namespace can
{

constexpr uint32_t CAN_EFF_MASK = 0x1FFFFFFFU;       /**< Bit mask for 29-bit IDs */
constexpr uint32_t CAN_SFF_MASK = 0x7FFU;            /**< Bit mask for 11-bit IDs */
constexpr uint32_t CAN_EFF_UPPER_MASK = 0x1FFFF800U; /**< Upper 18 bits of 29-bit extended ID */

class CanFrame
{
  public:
    using FrameId = uint64_t;
    class Timestamp
    {
      public:
        constexpr Timestamp(int64_t s = 0, int64_t ns = 0) noexcept : _seconds(s), _nanoseconds(ns)
        {
        }
        constexpr static Timestamp fromNanoseconds(uint64_t ns) noexcept
        {
            return Timestamp(ns / 1000000000, ns % 1000000000);
        }
        constexpr static Timestamp fromMicroseconds(uint64_t us) noexcept
        {
            return Timestamp(us / 1000000, us % 1000000);
        }

        constexpr int64_t seconds() const noexcept
        {
            return _seconds;
        }
        constexpr int64_t nanoseconds() const noexcept
        {
            return _nanoseconds;
        }

      private:
        int64_t _seconds;
        int64_t _nanoseconds;
    };

    enum FrameType {
        UnknownFrame = 0,
        DataFrame = 1,
        ErrorFrame = 2,
        RemoteRequestFrame = 3,
        InvalidFrame = 4
    };

    explicit CanFrame(FrameType type = DataFrame) noexcept
        : _isExtendedId(false), _isErrorState(false), _isFD(false), _isBRS(false), _isEcho(false)
    {
        setId(0);
        setFrameType(type);
    }

    enum FrameError {
        NoError = 0,
        TxTimeoutError = (1 << 0),
        ArbitrationLostError = (1 << 1),
        ControllerError = (1 << 2),
        ProtocolError = (1 << 3),
        TransceiverError = (1 << 4),
        NoAckError = (1 << 5),
        BusOffError = (1 << 6),
        BusError = (1 << 7),
        ControllerRestart = (1 << 8),
        UnknownError = (1 << 9),
        AnyError = CAN_EFF_MASK,
    };

    explicit CanFrame(CanFrame::FrameId id, const std::vector<std::byte> &data)
        : _type(DataFrame),
          _isExtendedId(false),
          _isErrorState(false),
          _isFD(data.size() > 8),
          _isBRS(0),
          _isEcho(0),
          _payload(data)
    {
        setId(id);
    }

    bool isValid() const noexcept
    {
        if (_type == InvalidFrame)
            return false;

        // Check for 29-bit ID used with 11-bit frame
        if (!_isExtendedId && (_id & CAN_EFF_UPPER_MASK))
            return false;

        if (!_isValidId)
            return false;

        const size_t len = _payload.size();
        if (_isFD) {
            // FD frames can have 8, 12, 16, 20, 24, 32, 48, or 64 bytes
            if (_type == RemoteRequestFrame)
                return false;  // FD doesn't support RTR
            return len <= 8 || len == 12 || len == 16 || len == 20 || len == 24 || len == 32 || len == 48 || len == 64;
        }
        // Non-FD frames support up to 8 bytes
        return len <= 8;
    }

    constexpr FrameType frameType() const noexcept
    {
        return _type;
    }
    constexpr void setFrameType(FrameType newType) noexcept
    {
        switch (newType) {
            case DataFrame:
            case ErrorFrame:
            case RemoteRequestFrame:
            case UnknownFrame:
            case InvalidFrame:
                _type = newType;
                return;
        }
    }

    constexpr bool isExtendedId() const noexcept
    {
        return _isExtendedId;
    }
    constexpr void setExtendedId(bool isExtended) noexcept
    {
        _isExtendedId = isExtended;
    }

    constexpr CanFrame::FrameId id() const noexcept
    {
        if (_type == ErrorFrame) {
            return 0;
        }
        return _id & CAN_EFF_MASK;
    }

    constexpr void setId(CanFrame::FrameId newId)
    {
        if (newId <= CAN_EFF_MASK) {
            _isValidId = true;
            _id = newId;
            setExtendedId(_isExtendedId || (newId & CAN_EFF_UPPER_MASK));
        } else {
            _isValidId = false;
            _id = 0;
        }
    }

    std::vector<std::byte> payload() const
    {
        return _payload;
    }
    void setPayload(const std::vector<std::byte> &data)
    {
        _payload = data;
        if (data.size() > 8)
            _isFD = 0x1;
    }

    constexpr Timestamp timestamp() const noexcept
    {
        return _timestamp;
    }
    constexpr void setTimestamp(Timestamp ts) noexcept
    {
        _timestamp = ts;
    }

    constexpr FrameError error() const noexcept
    {
        if (_type != ErrorFrame)
            return NoError;

        return FrameError(_id & AnyError);
    }

    constexpr void setError(FrameError err)
    {
        if (_type != ErrorFrame)
            return;

        _id = (err & AnyError);
    }

    constexpr bool isFD() const noexcept
    {
        return _isFD;
    };

    constexpr void setFD(bool isFD) noexcept
    {
        _isFD = isFD;
        if (!isFD) {
            _isBRS = false;
            _isErrorState = false;
        }
    }

    constexpr bool isBitrateSwitch() const noexcept
    {
        return _isBRS;
    }

    constexpr void setBitrateSwitch(bool brs) noexcept
    {
        _isBRS = brs;
        if (brs)
            _isFD = true;
    }

    constexpr bool isErrorState() const noexcept
    {
        return _isErrorState;
    }

    constexpr void setErrorState(bool es) noexcept
    {
        _isErrorState = es;
    }

    constexpr bool isLocalEcho() const noexcept
    {
        return _isEcho;
    }

    constexpr void setLocalEcho(bool echo) noexcept
    {
        _isEcho = echo;
    }

  private:
    uint64_t _id : 29;
    enum FrameType _type;

    bool _isExtendedId; /**< If set, frame uses 29-bit extended id */
    bool _isErrorState; /**< If set, frame is an error indicator */
    bool _isFD;         /**< If set, frame is a CAN FD frame */
    bool _isBRS;        /**< Bitrate switch */
    bool _isEcho;       /**< Local echo */
    bool _isValidId;    /**< If set, ID is valid */
    uint64_t _error;    /**< Error flags */

    std::vector<std::byte> _payload; /**< Data contents of frame */
    Timestamp _timestamp;            /**< Time message was received or transmitted */
};

}  // namespace can
}  // namespace net
}  // namespace datapanel

template <> struct fmt::formatter<datapanel::net::can::CanFrame::Timestamp> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(datapanel::net::can::CanFrame::Timestamp &ts, FormatContext &ctx) const
    {
        std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<int>> tp_seconds{
            std::chrono::duration<int>{ts.seconds()}};
        return fmt::format_to(ctx.out(), "{}.{:09d}", tp_seconds, ts.nanoseconds());
    }
};

template <> struct fmt::formatter<datapanel::net::can::CanFrame> : fmt::formatter<string_view> {
    template <typename FormatContext> auto format(datapanel::net::can::CanFrame &frame, FormatContext &ctx) const
    {
        switch (frame.frameType()) {
            case datapanel::net::can::CanFrame::FrameType::InvalidFrame:
                return fmt::format_to(ctx.out(), "[INVALID FRAME]");
            case datapanel::net::can::CanFrame::FrameType::DataFrame:
                return fmt::format_to(ctx.out(), "{} 0x{:0{}X}  [{}] {}", frame.timestamp(), frame.id(),
                                      frame.isExtendedId() ? 8 : 3, frame.payload().size(),
                                      datapanel::util::hexdump(frame.payload()));
            case datapanel::net::can::CanFrame::FrameType::ErrorFrame:
                return fmt::format_to(ctx.out(), "[ERROR FRAME]");
            case datapanel::net::can::CanFrame::FrameType::RemoteRequestFrame:
                return fmt::format_to(ctx.out(), "{} 0x{:0{}X}r [{}]", frame.timestamp(), frame.id(),
                                      frame.isExtendedId() ? 8 : 3, frame.payload().size());
            case datapanel::net::can::CanFrame::FrameType::UnknownFrame:
            default:
                break;
        }
        return fmt::format_to(ctx.out(), "[UNKNOWN FRAME]");
    }
};
