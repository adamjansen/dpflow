#pragma once

#include <vector>
#include <cstdint>

#include "ba/bytearray.hpp"
#include "ba/bytearray_view.hpp"

namespace datapanel
{

namespace dploader
{
constexpr std::byte SOH{0x01};
constexpr std::byte EOT{0x04};
constexpr std::byte DLE{0x10};

enum class Command {
    ReadBootInfo = 1,
    EraseFlash = 2,
    ProgramFlash = 3,
    ReadCrc = 4,
    JumpToApp = 5,
    ReadOemInfo = 6,
    ReadAppInfo = 7
};
uint16_t crc16(::ba::bytearray_view data, uint16_t start = 0x0000);
std::vector<std::byte> escape(std::vector<std::byte> &data);
std::vector<std::byte> unescape(std::vector<std::byte> &data);
std::vector<std::byte> decode(std::vector<std::byte> &frame);
std::vector<std::byte> encode(Command cmd);
std::vector<std::byte> encode(Command cmd, std::vector<std::byte> &payload);
class DPLoader
{
  private:
    void _request(Command cmd, std::vector<std::byte> &data, int timeoutMs = 1000);
};
}  // namespace dploader
}  // namespace datapanel
