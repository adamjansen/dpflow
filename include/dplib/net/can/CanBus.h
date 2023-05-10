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
 * @file CanBus.h
 * @author ajansen
 * @date 2023-04-26
 */

#pragma once

#include <string>
#include <list>

#include <spdlog/spdlog.h>

#include "dplib/net/can/CanInterface.h"

namespace datapanel
{
namespace net
{
namespace can
{

  /**
   * @brief CAN backend driver information
   */
class CanBusPluginInfo
{
  public:
    CanBusPluginInfo() {}
    CanBusPluginInfo(std::string n, std::unique_ptr<CanInterface> (*c)(const std::string &channel),
                     std::list<CanInterfaceInfo> (*s)())
        : name(n), create(c), scan(s)
    {
    }
    /** Name of backend driver */
    std::string name;
    /** Factory function */
    std::unique_ptr<CanInterface> (*create)(const std::string &channel);
    /** Probe for available interfaces */
    std::list<CanInterfaceInfo> (*scan)();
};

/**
 * @brief CAN Bus
 *
 * Handles connecting to a CAN bus and transmission and reception of frames.
 *
 * Users can detect and connect to available CAN interfaces.
 */
class CanBus
{
  public:
    /**
     * @brief Initialize the backend driver
     *
     * @param[in] plugin Name of backend driver (case-sensitive)
     * @param[in] channel Name of channel to connect to
     *
     * @return CanInterface object
     */
    static std::unique_ptr<CanInterface> create(const std::string &plugin, const std::string &channel)
    {
        auto &pinfo = getPluginRegistry()[plugin];
        return pinfo.create(channel);
    }

  /**
   * @brief Register a backend for use with create
   *
   * @param info Backend plugin information
   *
   * @return true if the plugin was registered successfully
   */
  static bool registerPlugin(const CanBusPluginInfo &info);

  /**
   * @brief Probe for available CAN channels.
   *
   * Backends may support multiple channels.  These channels may or
   * may not be on the same physical device.
   *
   * @note Some backends may not be able to detect all channels.
   *
   * @return available channel names
   */
    static std::list<CanInterfaceInfo> availableChannels();

  private:
    static std::map<std::string, CanBusPluginInfo> &getPluginRegistry();

};
}  // namespace can
}  // namespace net
}  // namespace datapanel
