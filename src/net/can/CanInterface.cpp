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
 * @date 2023-04-27
 */

#include <string>
#include <sstream>
#include <chrono>

#include <fmt/core.h>
#include <fmt/chrono.h>

#include "dplib/net/can/CanInterface.h"

using namespace datapanel::net::can;

CanInterface::CanBusError CanInterface::error() const
{
    return _lastError;
}

std::string CanInterface::errorMessage() const
{
    return (_lastError == CanInterface::NoError) ? std::string() : _errorMessage;
}

void CanInterface::setError(const std::string &errorMessage, CanBusError error)
{
    _errorMessage = errorMessage;
    _lastError = error;
}

void CanInterface::clearError()
{
    _errorMessage.clear();
    _lastError = CanBusError::NoError;
}

size_t CanInterface::countRxPending() const
{
    return _rxFrames.size();
}

size_t CanInterface::countTxPending() const
{
    return _txFrames.size();
}

void CanInterface::enqueueRxFrames(const std::list<CanFrame> &frames)
{
    std::lock_guard<std::mutex> guard(_rxLock);
    _rxFrames.insert(_rxFrames.end(), frames.begin(), frames.end());
}

void CanInterface::enqueueTxFrame(const CanFrame &frame)
{
    _txFrames.push_back(frame);
}

CanFrame CanInterface::dequeueTxFrame()
{
    if (_txFrames.empty())
        return CanFrame(CanFrame::InvalidFrame);
    return _txFrames.front();
}

void CanInterface::setConfigOption(ConfigOption opt, const ConfigOptionValue &value)
{
    if (_configOptions.count(opt) > 0) {
        _configOptions[opt] = value;
    }
}

CanInterface::ConfigOptionValue CanInterface::configOption(ConfigOption opt) const
{
    const auto it = _configOptions.find(opt);
    if (it == _configOptions.end())
        return CanInterface::ConfigOptionValue();
    return (*it).second;
}

std::list<CanInterface::ConfigOption> CanInterface::configOptions() const
{
    std::list<CanInterface::ConfigOption> opts;
    for (const auto &[key, value] : _configOptions) opts.push_back(key);

    return opts;
}

bool CanInterface::restart()
{
    return false;
}

CanInterface::CanBusState CanInterface::busStatus()
{
    return CanInterface::CanBusState::Unknown;
}

void CanInterface::flushTx()
{
    _txFrames.clear();
}

void CanInterface::flushRx()
{
    std::lock_guard<std::mutex> guard(_rxLock);
    _rxFrames.clear();
}

CanFrame CanInterface::recv()
{
    if (_state != ConnectedState) {
        setError("Cannot receive while interface is disconnected", CanInterface::OperationError);
        return CanFrame(CanFrame::InvalidFrame);
    }

    clearError();

    std::lock_guard<std::mutex> guard(_rxLock);
    if (_rxFrames.empty()) {
        return CanFrame(CanFrame::InvalidFrame);
    }

    return _rxFrames.front();
}

std::list<CanFrame> CanInterface::recvAll()
{
    if (_state != ConnectedState) {
        setError("Cannot receive while interface is disconnected", CanInterface::OperationError);
        return std::list<CanFrame>();
    }
    clearError();

    std::lock_guard<std::mutex> guard(_rxLock);
    std::list<CanFrame> frames;
    frames.swap(_rxFrames);
    return frames;
}

bool CanInterface::connect()
{
    if (_state != DisconnectedState) {
        const std::string message = "Disconnect before connecting";
        setError(message, CanInterface::ConnectionError);
        return false;
    }

    setState(ConnectionPendingState);
    if (!open()) {
        setState(DisconnectedState);
        return false;
    }

    clearError();

    return true;
}

void CanInterface::disconnect()
{
    if ((_state == DisconnectedState) || (_state == DisconnectPendingState)) {
        // warn: can't disconnect unconnected interface
        return;
    }
    setState(DisconnectPendingState);
    close();
}

CanInterface::CanConnectionState CanInterface::state() const
{
    return _state;
}

void CanInterface::setState(CanInterface::CanConnectionState newState)
{
    if (newState == _state)
        return;

    _state = newState;
}
