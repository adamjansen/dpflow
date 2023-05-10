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
 * @file CanFrame.h
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

/**
 * @brief Represent a single CAN message frame
 */
class CanFrame
{
  public:
    using FrameId = uint64_t;

    /**
     * @brief Moment in time when the frame was received or transmitted
     *
     * The time is stored as an offset from the Unix epoch (roughly midnight, 1970-01-01).
     */
    class Timestamp
    {
      public:
        constexpr Timestamp(int64_t s = 0, int64_t ns = 0) noexcept : _seconds(s), _nanoseconds(ns)
        {
        }

        /**
         * @brief Create a timesamp from a number of nanoseconds
         *
         * @param[in] ns Nanoseconds since epoch
         *
         * @return A new timestamp
         */
        constexpr static Timestamp fromNanoseconds(uint64_t ns) noexcept
        {
            return Timestamp(ns / 1000000000, ns % 1000000000);
        }

        /**
         * @brief Create a timestamp from a number of microseconds
         *
         * @param[in] us Microseconds since epoch
         *
         * @return A new timestamp
         */
        constexpr static Timestamp fromMicroseconds(uint64_t us) noexcept
        {
            return Timestamp(us / 1000000, us % 1000000);
        }

        /**
         * @brief Integer portion of timestamp
         *
         * @return Number of integer seconds
         */
        constexpr int64_t seconds() const noexcept
        {
            return _seconds;
        }

        /**
         * @brief Fractional portion of timestamp
         *
         * @return Number of nanoseconds
         */
        constexpr int64_t nanoseconds() const noexcept
        {
            return _nanoseconds;
        }

      private:
        int64_t _seconds;
        int64_t _nanoseconds;
    };

    /**
     * @brief Indicate type of CAN frame
     */
    enum FrameType {
        UnknownFrame = 0,       /**< Frame type cannot be identified */
        DataFrame = 1,          /**< Typical data frame */
        ErrorFrame = 2,         /**< Error frame (no payload) */
        RemoteRequestFrame = 3, /**< RTR frame (no payload) */
        InvalidFrame = 4,       /**< Frame is not valid */
    };

    explicit CanFrame(FrameType type = DataFrame) noexcept
        : _isExtendedId(false), _isErrorState(false), _isFD(false), _isBRS(false), _isEcho(false)
    {
        setId(0);
        setFrameType(type);
    }

    /**
     * @brief Bitmask representing possible frame errors
     */
    enum FrameError {
        NoError = 0,                     /**< No errors in frame */
        TxTimeoutError = (1 << 0),       /**< Transmit not completed in time */
        ArbitrationLostError = (1 << 1), /**< Transmit aborted due to lost arbitration */
        ControllerError = (1 << 2),      /**< Device-specific error */
        ProtocolError = (1 << 3),        /**< Protocol-specific error */
        TransceiverError = (1 << 4),     /**< Transceiver problem */
        NoAckError = (1 << 5),           /**< Frame was not acknowledged */
        BusOffError = (1 << 6),          /**< BUS_OFF condition */
        BusError = (1 << 7),             /**< Bus error */
        ControllerRestart = (1 << 8),    /**< Controller restarting */
        UnknownError = (1 << 9),         /**< Some other error */
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

    /**
     * @brief Check if a frame is valid
     *
     * Valid frames:
     *
     * 1. Do not have type InvalidFrame
     * 2. Have extended ID bit set if identifier is larger than 0x7FF
     * 3. Do not have an invalid identifier
     * 4. Have a frame size of 8, 12, 16, 20, 24, 32, 48, or 64 bytes
     *    if the frame is a flexible data rate frame, or 8 or less if
     *    the frame is a classic CAN frame.
     * 5. Do not have the RTR bit set if the frame is a CANFD frame.
     * 6. All other frames are invalid.
     *
     * @return true if the frame is valid, or false if invalid
     */
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

    /**
     * @return the type of frame
     */
    constexpr FrameType frameType() const noexcept
    {
        return _type;
    }

    /**
     * @param[in] newType Frame type to change to
     */
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

    /**
     * @return true if the frame uses a 29-bit identifier
     */
    constexpr bool isExtendedId() const noexcept
    {
        return _isExtendedId;
    }

    /**
     * @brief Change to 11 or 29 bit ID
     *
     * @param isExtended extended ID flag
     */
    constexpr void setExtendedId(bool isExtended) noexcept
    {
        _isExtendedId = isExtended;
    }

    /**
     * @brief CAN identifier
     *
     * @return CAN identifier used by frame
     */
    constexpr CanFrame::FrameId id() const noexcept
    {
        if (_type == ErrorFrame) {
            return 0;
        }
        return _id & CAN_EFF_MASK;
    }

    /**
     * @brief Change CAN identifier
     *
     * If the identifier is greater than 0x7FF, the
     * frame will enable 29-bit identifiers.
     *
     * @param newId New CAN identifier
     */
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

    /**
     * @brief Data contents of frame
     *
     * @return Frame payload data
     */
    std::vector<std::byte> payload() const
    {
        return _payload;
    }

    /**
     * @brief Change payload
     *
     * @param[in] data New frame payload data
     */
    void setPayload(const std::vector<std::byte> &data)
    {
        _payload = data;
        if (data.size() > 8)
            _isFD = 0x1;
    }

    /**
     * @brief Time frame was received
     *
     * @return Timestamp of frame
     */
    constexpr Timestamp timestamp() const noexcept
    {
        return _timestamp;
    }

    /**
     * @brief Change timestamp
     *
     * @param ts New timestamp
     */
    constexpr void setTimestamp(Timestamp ts) noexcept
    {
        _timestamp = ts;
    }

    /**
     * @brief Get error flags
     *
     * @return Bitmask indicating errors present in frame
     */
    constexpr FrameError error() const noexcept
    {
        if (_type != ErrorFrame)
            return NoError;

        return FrameError(_id & AnyError);
    }

    /**
     * @brief Set error flags
     *
     * @param err Bitmask indicating errors
     */
    constexpr void setError(FrameError err)
    {
        if (_type != ErrorFrame)
            return;

        _id = (err & AnyError);
    }

    /**
     * @brief Determine if frame is CAN FD frame
     *
     * @return true if frame uses Flexible Data Rate
     */
    constexpr bool isFD() const noexcept
    {
        return _isFD;
    };

    /**
     * @brief Set FD flag
     *
     * @param[in] isFD enable or disable CANFD for this frame
     */
    constexpr void setFD(bool isFD) noexcept
    {
        _isFD = isFD;
        if (!isFD) {
            _isBRS = false;
            _isErrorState = false;
        }
    }

    /**
     * @brief Check if the frame uses the bitrate switch flag
     *
     * @return True if the frame uses CANFD's bitrate switch
     */
    constexpr bool isBitrateSwitch() const noexcept
    {
        return _isBRS;
    }

    /**
     * @brief Change bitrate switch flag
     *
     * If set, the frame will be marked as an FD frame.
     *
     * @param[in] brs Change bitrate switch flag
     */
    constexpr void setBitrateSwitch(bool brs) noexcept
    {
        _isBRS = brs;
        if (brs)
            _isFD = true;
    }

    /**
     * @brief Check for error state
     *
     * @return true if an error state is present
     */
    constexpr bool isErrorState() const noexcept
    {
        return _isErrorState;
    }

    /**
     * @brief change error state
     * @param[in] es New error state
     */
    constexpr void setErrorState(bool es) noexcept
    {
        _isErrorState = es;
    }

    /**
     * @brief Check if a frame is a local echo
     *
     * @return true if the frame was sent from another application on
     *         this interface
     */
    constexpr bool isLocalEcho() const noexcept
    {
        return _isEcho;
    }

    /**
     * @brief Change local echo flag
     *
     * @param[in] echo Change echo flag
     */
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

/** @cond formatters */
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
/** @endcond */
