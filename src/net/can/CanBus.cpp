#include <string>
#include <list>
#include <map>
#include <iostream>

#include "dplib/net/can/CanBus.h"

using namespace datapanel::net::can;

std::map<std::string, CanBusPluginInfo> &CanBus::getPluginRegistry()
{
    static std::map<std::string, CanBusPluginInfo> plugin_registry;
    return plugin_registry;
}

bool CanBus::registerPlugin(const CanBusPluginInfo &info)
{
    if (getPluginRegistry().find(info.name) != getPluginRegistry().end()) {
        return false;
    }
    getPluginRegistry()[info.name] = info;
    return true;
}
std::list<CanInterfaceInfo> CanBus::availableChannels()
{
    std::list<CanInterfaceInfo> channels;

    for (const auto &[name, info] : CanBus::getPluginRegistry()) {
        for (const auto &c : info.scan()) channels.push_back(c);
    }
    return channels;
}
