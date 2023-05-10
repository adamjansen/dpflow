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

#include <list>
#include <variant>
#include <map>
#include <mutex>

#include "dplib/net/can/CanFrame.h"

#include <fmt/format.h>
#include <magic_enum.hpp>

namespace datapanel
{
namespace net
{
namespace can
{

class CanInterfaceInfo
{
  public:
    CanInterfaceInfo()
    {
    }
    std::string plugin;
    std::string name;
    std::string description;
    bool supportsFD;
    int currentBitrate;
};

class CanInterface
{
  public:
    enum CanBusError {
        NoError,
        RxError,
        TxError,
        ConnectionError,
        ConfigurationError,
        UnknownError,
        OperationError,
        TimeoutError
    };
    enum CanConnectionState {
        DisconnectedState,
        ConnectionPendingState,
        ConnectedState,
        DisconnectPendingState,
    };

    enum class CanBusState {
        Unknown,
        OK,
        Warning,
        Error,
        BusOff,
    };

    enum ConfigOption {
        CfgOptLoopback,
        CfgOptRxOwn,
        CfgOptBitrate,
        CfgOptFD,
        CfgOptOther,
    };

    using ConfigOptionValue = std::variant<int, double, bool, std::string>;

    CanInterface()
    {
    }
    virtual ~CanInterface() = default;

    virtual void setConfigOption(ConfigOption opt, const ConfigOptionValue &value);
    ConfigOptionValue configOption(ConfigOption opt) const;
    std::list<ConfigOption> configOptions() const;

    virtual bool send(const CanFrame &frame) = 0;
    virtual CanFrame recv();
    virtual std::list<CanFrame> recvAll();
    virtual bool restart();

    bool connect();
    void disconnect();

    virtual CanConnectionState state() const;

    virtual CanBusState busStatus();

    CanBusError error() const;
    std::string errorMessage() const;

    size_t countRxPending() const;
    size_t countTxPending() const;

    void flushTx();
    void flushRx();

    static std::list<std::string> availableChannels();

  protected:
    void setState(CanConnectionState state);
    void setError(const std::string &errorMessage, CanBusError);
    void clearError();

    void enqueueRxFrames(const std::list<CanFrame> &frames);
    void enqueueTxFrame(const CanFrame &frame);
    CanFrame dequeueTxFrame();
    bool pendingTxFrames() const;

    virtual bool open() = 0;
    virtual bool close() = 0;

  private:
    std::list<CanFrame> _rxFrames;
    std::mutex _rxLock;
    std::list<CanFrame> _txFrames;

    std::map<ConfigOption, ConfigOptionValue> _configOptions;

    CanBusError _lastError = CanBusError::NoError;
    CanConnectionState _state = CanConnectionState::DisconnectedState;
    std::string _errorMessage;
};

template <typename OStream> OStream &operator<<(OStream &os, const CanInterface::CanBusState &state)
{
    return fmt::format_to(std::ostream_iterator<char>(os), "{}", magic_enum::enum_name(state));
}

}  // namespace can
}  // namespace net
}  // namespace datapanel

template <> struct fmt::formatter<datapanel::net::can::CanInterface::CanBusState> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(datapanel::net::can::CanInterface::CanBusState state, FormatContext &ctx) const
    {
        string_view name = magic_enum::enum_name(state);
        return formatter<string_view>::format(name, ctx);
    }
};

template <> struct fmt::formatter<datapanel::net::can::CanInterface::CanBusError> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(datapanel::net::can::CanInterface::CanBusError error, FormatContext &ctx) const
    {
        string_view name = magic_enum::enum_name(error);
        return formatter<string_view>::format(name, ctx);
    }
};

template <> struct fmt::formatter<datapanel::net::can::CanInterface::CanConnectionState> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(datapanel::net::can::CanInterface::CanConnectionState state, FormatContext &ctx) const
    {
        string_view name = magic_enum::enum_name(state);
        return formatter<string_view>::format(name, ctx);
    }
};

template <> struct fmt::formatter<datapanel::net::can::CanInterface::ConfigOption> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(datapanel::net::can::CanInterface::ConfigOption opt, FormatContext &ctx) const
    {
        string_view name = magic_enum::enum_name(opt);
        return formatter<string_view>::format(name, ctx);
    }
};

template <> struct fmt::formatter<datapanel::net::can::CanInterface::ConfigOptionValue> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(const datapanel::net::can::CanInterface::ConfigOptionValue& v, FormatContext &ctx) const
    {
        if (const int* pval = std::get_if<int>(&v))
            return fmt::format_to(ctx.out(), "{}", *pval);
        else if (const double* pval = std::get_if<double>(&v))
            return fmt::format_to(ctx.out(), "{}", *pval);
        else if (const bool* pval = std::get_if<bool>(&v))
            return fmt::format_to(ctx.out(), "{}", *pval ? "true" : "false");
        else if (const std::string *pval = std::get_if<std::string>(&v))
            return fmt::format_to(ctx.out(), "{}", *pval);
        else
            return fmt::format_to(ctx.out(), "<invalid>");
    }
};
