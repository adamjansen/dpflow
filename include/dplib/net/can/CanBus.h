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

class CanBusPluginInfo
{
  public:
    CanBusPluginInfo() {}
    CanBusPluginInfo(std::string n, std::unique_ptr<CanInterface> (*c)(const std::string &channel),
                     std::list<CanInterfaceInfo> (*s)())
        : name(n), create(c), scan(s)
    {
    }
    std::string name;
    std::unique_ptr<CanInterface> (*create)(const std::string &channel);
    std::list<CanInterfaceInfo> (*scan)();
};

class CanBus
{
  public:
    static std::unique_ptr<CanInterface> create(const std::string &plugin, const std::string &channel)
    {
        auto &pinfo = getPluginRegistry()[plugin];
        return pinfo.create(channel);
    }


  // call this function in a derived class from a static context
  static bool registerPlugin(const CanBusPluginInfo &info);

    static std::list<CanInterfaceInfo> availableChannels();

  private:
    static std::map<std::string, CanBusPluginInfo> &getPluginRegistry();

};
}  // namespace can
}  // namespace net
}  // namespace datapanel
