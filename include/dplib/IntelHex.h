#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <functional>

namespace datapanel
{
class IntelHex
{
  public:
    /**
     * @brief Intel HEX record type
     */
    enum class RecordType {
        Data = 0,                /**< Data record (memory contents) */
        EndOfFile = 1,           /**< End-of-file indicator */
        ExtSegmentAddress = 2,   /**< Set segment address */
        StartSegmentAddress = 3, /**< Start of segment address */
        ExtLinearAddress = 4,    /**< Set linear address */
        StartLinearAddress = 5,  /**< Start of linear address */
        Invalid = 0xFF,
    };

    enum class ReadState {
        ReadWait,
        ReadCountHigh,
        ReadCountLow,
        ReadAddressMsbHigh,
        ReadAddressMsbLow,
        ReadAddressLsbHigh,
        ReadAddressLsbLow,
        ReadRecordTypeHigh,
        ReadRecordTypeLow,
        ReadDataHigh,
        ReadDataLow,
        ReadChecksumHigh,
        ReadChecksumLow,
    };

    struct Record {
        RecordType type;
        uint16_t address;
        uint8_t length;
        std::vector<std::byte> data;
        uint8_t checksum;
        void clear()
        {
            data.clear();
            length = 0;
            type = RecordType::Invalid;
            address = 0x0000;
            checksum = 0x00;
        }
    };
    using Callback = std::function<int(Record &rec, uint8_t sum)>;

    IntelHex(Callback func, bool strict = false);

    void parse(uint8_t data);
    void parse(std::vector<std::byte> data);
    void parse(std::string data);

  private:
    ReadState m_state;
    Record m_rec;
    uint8_t m_pos;  // Byte index within record
    uint8_t m_pendingByte;
    Callback m_callback;

    bool m_strict; /**< If set, the start code (initial ':') is required */
};
}  // namespace datapanel
