#pragma once

#include <string>
#include <vector>

namespace datapanel {
namespace util {

std::string hexdump(const std::vector<std::byte> &data, const std::string& sep=" ");
}
} // namespace datapanel
