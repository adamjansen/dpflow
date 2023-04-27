#include <iostream>
#include <string>
#include "net/can/CanFrame.h"


using namespace datapanel::net::can;


std::ostream& operator<<(std::ostream& out, const CanId& id)
{
    return out << std::string(id);
}

std::ostream& operator<<(std::ostream& out, const CanFrame& frame)
{
    return out << "[Time] " << frame.getId() << " " << frame.getData();
}
