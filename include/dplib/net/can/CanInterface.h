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
 * @file CanInterface.h
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

#include <sigslot/signal.hpp>

namespace datapanel
{
namespace net
{
namespace can
{

/**
 * @brief Holds basic information about a CAN device
 */
class CanInterfaceInfo
{
  public:
    CanInterfaceInfo()
    {
    }
    std::string plugin;      /**< Backend's unique name. Case sensitive. */
    std::string name;        /**< Name of channel.  Must be unique within backend. */
    std::string description; /**< Arbitrary text set by backend */
    bool supportsFD;         /**< If set, CANFD is supported on this interface */
    int currentBitrate;      /**< Current bitrate of interface */
};

/**
 * @brief Base class for CAN devices
 *
 *
 * ## Configuration Options
 *
 * | Option | Type | Description |
 * | ------ | ----- | --------------- |
 * | @ref CfgOptLoopback | bool | Receive frames sent by other applications on this interface |
 * | @ref CfgOptRxOwn | bool | Receive frames transmitted via this interface |
 * | @ref CfgOptBitrate | int | Bitrate of CAN interface |
 * | @ref CfgOptFD | bool | If set, Flexible Data Rate is enabled |
 * | @ref CfgOptOther | | Interface-specific |
 *
 */
class CanInterface
{
  public:
    /**
     * @brief Possible error conditions for CanInterface operation
     */
    enum CanBusError {
        NoError,            /**< Normal operation */
        RxError,            /**< Receive error */
        TxError,            /**< Transmit error */
        ConnectionError,    /**< Could not connect */
        ConfigurationError, /**< Something is wrong with the configuration */
        UnknownError,       /**< Some other error */
        OperationError,     /**< Illegal operation */
        TimeoutError        /**< Operation timed out */
    };

    /**
     * @brief Logical interface status
     */
    enum CanConnectionState {
        DisconnectedState,      /**< Not connected to bus */
        ConnectionPendingState, /**< Connection in progress */
        ConnectedState,         /**< Connected, normal operation */
        DisconnectPendingState, /**< Disconnect in progress */
    };

    /**
     * @brief Represent the physical state of the CAN bus
     */
    enum class CanBusState {
        Unknown, /**< State cannot be determined */
        OK,      /**< Normal operation */
        Warning, /**< BUS_WARN condition */
        Error,   /**< BUS_ERROR condition */
        BusOff,  /**< BUS_OFF; transmission and reception is not possible */
    };

    /**
     * @brief Configuration options for CAN interfaces
     */
    enum ConfigOption {
        CfgOptLoopback, /**< When set, frames sent from other applications on this interface are received */
        CfgOptRxOwn,    /**< When set, frames sent from this interface are also received. */
        CfgOptBitrate,  /**< Data bitrate */
        CfgOptFD,       /**< If set, Flexible Data Rate support is enabled */
        CfgOptOther,    /**< Interface-specific option */
    };

    sigslot::signal<CanInterface::CanBusError> errorOccurred;
    sigslot::signal<CanInterface::CanConnectionState> connectionStateChanged;
    sigslot::signal<> framesReceived;
    sigslot::signal<> framesTransmitted;

    /**
     * @brief Used with ConfigOption to configure interfaces
     */
    using ConfigOptionValue = std::variant<int, double, bool, std::string>;

    CanInterface()
    {
    }
    virtual ~CanInterface() = default;

    /**
     * @brief Change a configuration option
     *
     * @note
     * Most interfaces only support changing options when the interface
     * is disconnected.
     *
     * @param[in] opt Option to set
     * @param[in] value Desired value for @p opt
     */
    virtual void setConfigOption(ConfigOption opt, const ConfigOptionValue &value);

    /**
     * @brief Get the current value of a configuration option
     *
     * @param[in] opt Option to get
     *
     * @return value of @p opt
     */
    ConfigOptionValue configOption(ConfigOption opt) const;

    /**
     * @brief Get a list of all supported options for the interface
     *
     * @return List of supported ConfigOption keys
     */
    std::list<ConfigOption> configOptions() const;

    /**
     * @brief Transmit a single CAN frame
     *
     * @param[in] frame Frame to transmit
     *
     * @return true if the transmission was successful
     */
    virtual bool send(const CanFrame &frame) = 0;

    /**
     * @brief Returns the most-recently received CAN frame
     *
     * @return Received frame, or CanFrame::InvalidFrame if no
     *         frame was available
     */
    virtual CanFrame recv();

    /**
     * @brief Get all messages in the receive buffer
     *
     * @return One or more received frames, or an empty list if no frames
     *         are available to receive
     */
    virtual std::list<CanFrame> recvAll();

    /**
     * @brief Restart the interface to clear an error
     *
     * Not all hardware interfaces support this.
     *
     * @return true if the interface successfully restarted
     */
    virtual bool restart();

    /**
     * @brief Connect to the CAN interface
     *
     * @return true on success, or false on failure
     *
     * @sa error
     * @sa errorMessage
     */
    bool connect();

    /**
     * @brief Terminate connection to CAN interface
     */
    void disconnect();

    /**
     * @brief Get current connection state
     *
     * @return current connection state
     */
    virtual CanConnectionState state() const;

    /**
     * @brief Get physical status of bus
     *
     * @note Not all devices support getting this information.
     *
     * @return Current physical status of bus, or CanBusState::Unknown
     *         if the status cannot be determined.
     */
    virtual CanBusState busStatus();

    /**
     * @brief Most-recent error condition
     *
     * @return error code
     * @sa errorMessage
     */
    CanBusError error() const;

    /**
     * @brief User-visible error message
     *
     * Error message corresponding to the code returned by error.
     *
     * @return Error message string
     *
     * @sa error
     */
    std::string errorMessage() const;

    /**
     * @brief Number of messages in receive buffer
     *
     * The number of messages in the receive buffer is the size of the
     * list that would be returned by recvAll().
     *
     * @return Number of available messages
     */
    size_t countRxPending() const;

    /**
     * @brief Number of messages in transmit queue
     *
     * The number of messages in the transmit queue is the size of
     * the list containing messages that have not been transmitted yet.
     *
     * @return The number of messages pending transmission
     */
    size_t countTxPending() const;

    /**
     * @brief Clear transmit buffer
     *
     * Remove all pending messages from the transmit queue.
     */
    void flushTx();

    /**
     * @brief Clear receive buffer
     *
     * Remove all received messages from the received queue.
     */
    void flushRx();

    /**
     * @brief Probe for supported channel names
     *
     * @return All detected channel names for this driver
     *
     * @note Some interface drivers may require manual
     *       configuration and do not support probing.
     */
    static std::list<std::string> availableChannels();

  protected:
    void setState(CanConnectionState state);
    void setError(const std::string &errorMessage, CanBusError);
    void clearError();

    void enqueueRxFrames(const std::list<CanFrame> &frames);
    void enqueueTxFrame(const CanFrame &frame);
    CanFrame dequeueTxFrame();
    bool pendingTxFrames() const;

    /**
     * @brief Initialize connection
     *
     * This method is called by connect().  Subclasses override
     * this method to implement the connection initialization.
     *
     * Subclasses should set useful error messages on failure.
     *
     * @return true on success, or false on failure
     *
     * @sa error
     * @sa errorMessage
     */
    virtual bool open() = 0;

    /**
     * @brief Disconnect
     *
     * This method is called by disconnect().  Subclasses override
     * this method to implement closing the connection.
     *
     * Subclasses should set useful error messages on failure.
     *
     * @return true on success, or false on failure
     *
     * @sa error
     * @sa errorMessage
     */
    virtual bool close() = 0;

  private:
    /** Incoming CanFrames */
    std::list<CanFrame> _rxFrames;
    /** Mutex to guard receive list */
    std::mutex _rxLock;
    /** Outgoing CanFrames */
    std::list<CanFrame> _txFrames;

    /** Supported options and their values */
    std::map<ConfigOption, ConfigOptionValue> _configOptions;

    CanBusError _lastError = CanBusError::NoError;
    CanConnectionState _state = CanConnectionState::DisconnectedState;
    std::string _errorMessage;
};


}  // namespace can
}  // namespace net
}  // namespace datapanel

/** @cond formatters */

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
    auto format(const datapanel::net::can::CanInterface::ConfigOptionValue &v, FormatContext &ctx) const
    {
        if (const int *pval = std::get_if<int>(&v))
            return fmt::format_to(ctx.out(), "{}", *pval);
        else if (const double *pval = std::get_if<double>(&v))
            return fmt::format_to(ctx.out(), "{}", *pval);
        else if (const bool *pval = std::get_if<bool>(&v))
            return fmt::format_to(ctx.out(), "{}", *pval ? "true" : "false");
        else if (const std::string *pval = std::get_if<std::string>(&v))
            return fmt::format_to(ctx.out(), "{}", *pval);
        else
            return fmt::format_to(ctx.out(), "<invalid>");
    }
};

/** @endcond */
