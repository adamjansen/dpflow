#include "dplib/IntelHex.h"

#include <cctype>

#include <magic_enum.hpp>

using namespace datapanel;

static uint8_t checksum(IntelHex::Record& rec) {
    uint8_t sum = rec.length + (rec.address >> 8) + (rec.address &0xFF) + static_cast<uint8_t>(rec.type);
    for (int i = 0; i < rec.length; i++)
        sum += static_cast<uint8_t>(rec.data[i]);
    sum = ~sum + 1;
    return sum;
}

IntelHex::IntelHex(IntelHex::Callback callback, bool strict) :
m_callback(callback), m_strict(strict), m_state(ReadState::ReadWait), m_pos(0), m_pendingByte(0)
{
    m_rec.clear();
}

void IntelHex::parse(std::vector<std::byte> data) {
    for (auto b: data)
        parse(static_cast<uint8_t>(b));
}

void IntelHex::parse(std::string data) {
    for (auto b: data)
        parse(static_cast<uint8_t>(b));
}

void IntelHex::parse(uint8_t data) {
    uint8_t b = data;

    if (::isdigit(data))
        b -= '0';
    else if (::isxdigit(data))
        b = toupper(data) - 'A' + 10;
    else if (data == ':') {
        m_pos = 0;
        m_state = ReadState::ReadCountHigh;
        m_rec.clear();
        m_pendingByte = 0;
        return;
    } else {
        /* Some other character */
        return;
    }

    uint8_t b2 = m_pendingByte + b;

    switch (m_state) {
        case ReadState::ReadWait:
            break;
        case ReadState::ReadCountHigh:
            m_state = ReadState::ReadCountLow;
            m_pendingByte = b << 4;
            break;
        case ReadState::ReadAddressMsbHigh:
            m_state = ReadState::ReadAddressMsbLow;
            m_pendingByte = b << 4;
            break;
        case ReadState::ReadAddressLsbHigh:
            m_state = ReadState::ReadAddressLsbLow;
            m_pendingByte = b << 4;
            break;
        case ReadState::ReadRecordTypeHigh:
            m_state = ReadState::ReadRecordTypeLow;
            m_pendingByte = b << 4;
            break;
        case ReadState::ReadDataHigh:
            m_state = ReadState::ReadDataLow;
            m_pendingByte = b << 4;
            break;
        case ReadState::ReadChecksumHigh:
            m_state = ReadState::ReadChecksumLow;
            m_pendingByte = b << 4;
            break;
        case ReadState::ReadCountLow:
            m_rec.length = b2;
            // TODO: at what point do we say a record is too long?
            m_state = ReadState::ReadAddressMsbHigh;
            break;
        case ReadState::ReadAddressMsbLow:
            m_rec.address = b2 << 8;
            m_state = ReadState::ReadAddressLsbHigh;
            break;
        case ReadState::ReadAddressLsbLow:
            m_rec.address += b2;
            m_state = ReadState::ReadRecordTypeHigh;
            break;
        case ReadState::ReadRecordTypeLow:
            m_rec.type = static_cast<RecordType>(b2);
            m_state = (m_rec.length > 0) ? ReadState::ReadDataHigh : ReadState::ReadChecksumHigh;
            m_pos = 0;
            break;
        case ReadState::ReadDataLow:
            m_rec.data.push_back(std::byte(b2));
            m_pos++;
            m_state = (m_pos >= m_rec.length) ? ReadState::ReadChecksumHigh : ReadState::ReadDataHigh;
            break;
        case ReadState::ReadChecksumLow:
            m_rec.checksum = b2;
            m_state = m_strict ? ReadState::ReadWait : ReadState::ReadCountHigh;
            uint8_t calculatedChecksum = checksum(m_rec);
            m_callback(m_rec, calculatedChecksum);
            break;
    }

}
