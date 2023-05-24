#include <doctest/doctest.h>
#include <dplib/IntelHex.h>

#include <iostream>

using datapanel::IntelHex;

TEST_CASE("ihex-record-eof")
{
    bool called = false;
    IntelHex ihex([&called](IntelHex::Record& rec, uint8_t sum) { 
        CHECK(rec.type == IntelHex::RecordType::EndOfFile);
        CHECK(rec.length == 0);
        CHECK(rec.address == 0x0000);
        CHECK(sum == rec.checksum); called = true; return 0; 
    });

    ihex.parse(":00000001FF");
    CHECK(called == true);
}

TEST_CASE("ihex-record-data")
{
    bool called = false;
    IntelHex ihex([&called](IntelHex::Record& rec, uint8_t sum) { 
        CHECK(rec.type == IntelHex::RecordType::Data);
        CHECK(rec.length == 16);
        CHECK(rec.address == 0x0130);
        CHECK(sum == rec.checksum); called = true; return 0; 
    });

    ihex.parse(":100130003F0156702B5E712B722B732146013421C7");
    CHECK(called == true);
}

TEST_CASE("ihex-record-checksum")
{
    bool called = false;
    IntelHex ihex([&called](IntelHex::Record& rec, uint8_t sum) { 
        CHECK(rec.type == IntelHex::RecordType::EndOfFile);
        CHECK(rec.length == 0);
        CHECK(rec.address == 0x0000);
        CHECK(sum == 0xFF); 
        CHECK(rec.checksum == 0xFF);
        called = true; return 0; 
    });

    ihex.parse(":00000001FF");
    CHECK(called == true);
}

TEST_CASE("ihex-record-bad-checksum")
{
    bool called = false;
    IntelHex ihex([&called](IntelHex::Record& rec, uint8_t sum) { 
        CHECK(rec.type == IntelHex::RecordType::EndOfFile);
        CHECK(rec.length == 0);
        CHECK(rec.address == 0x0000);
        CHECK(rec.checksum == 0xAA);
        CHECK(sum == 0xFF); 
        called = true; return 0; 
    });

    ihex.parse(":00000001AA");
    CHECK(called == true);
}
