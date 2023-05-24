#include <algorithm>
#include <dplib/dploader.h>

using namespace datapanel::dploader;

uint16_t crc16(std::vector<std::byte> &data, uint16_t start = 0x0000)
{
    uint16_t crc = start;
    for (auto b: data) {
        crc ^= static_cast<uint16_t>(b) << 8;
        for ([[maybe_unused]] int n:  {0, 1, 2, 3, 4, 5, 6, 7})
            if ((crc & 0x8000) > 0) 
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
    }
    return crc & 0xFFFF;
}

std::vector<std::byte> escape(std::byte b) {
    std::vector<std::byte> escaped;
  if ((b == SOH)  || (b == EOT) || (b == DLE)) 
        escaped.push_back(DLE);
    escaped.push_back(b);
    return escaped;
}

std::vector<std::byte> escape(std::vector<std::byte> &data)
{
    std::vector<std::byte> escaped;
    for (auto c: data) {
        escaped += escape(c);
    }
    return escaped;
}

std::vector<std::byte> unescape(std::vector<std::byte> &data) {
    std::vector<std::byte> unescaped;
    for (auto it=data.cbegin(); it != data.cend(); it++) {
        if (*it == DLE)
            it++; // Skip over escape character
        unescaped.push_back(*it);
    }
    return unescaped;
}
std::vector<std::byte> decode(std::vector<std::byte> &data) {
    std::vector<std::byte> unescaped = unescape(data);
    auto end = std::find(unescaped.rbegin(), unescaped.rend(), EOT);
    if (end == unescaped.rend()) {
        // TODO: Incomplete frame... exception?
    }
    if ((unescaped[0] != SOH) || (*end != EOT)) {
        // TODO: Invalid frame... exception?
    }
    auto length = end - unescaped.rbegin();
    unescaped.resize(length);

    std::vector<std::byte> payload(unescaped.begin(), unescaped.end() - 2);
    uint16_t crc_rx = static_cast<uint8_t>(unescaped[length - 3]) + (static_cast<uint8_t>(unescaped[length - 2]) << 8);
    auto crc = crc16(payload);
    if (crc_rx != crc) {
        // TODO: bad checksum... exception?
    }

    return unescaped;
}

std::vector<std::byte> encode(Command cmd) {
    std::vector<std::byte> empty;
    return encode(cmd, empty);
}

std::vector<std::byte> encode(Command cmd, std::vector<std::byte> &payload) {
    std::vector<std::byte> encoded;
    encoded.push_back(SOH);
    encoded.push_back(static_cast<std::byte>(cmd));

}
