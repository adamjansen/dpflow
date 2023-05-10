#include <doctest/doctest.h>
#include <dplib/util/hexdump.h>

#include <vector>

using datapanel::util::hexdump;

TEST_CASE("hexdump-DEADBEEF")
{
    std::vector<std::byte> data{std::byte(222), std::byte(173), std::byte(190), std::byte(239)};

    CHECK(hexdump(data) == "DE AD BE EF");
}

TEST_CASE("hexump-blank")
{
    std::vector<std::byte> data{};

    CHECK(hexdump(data) == "");
}

TEST_CASE("hexdump-custom-sep")
{
    std::vector<std::byte> data{std::byte(0x11), std::byte(0x22), std::byte(0x33), std::byte(0x44), std::byte(0x55)};
    CHECK(hexdump(data, ":") == "11:22:33:44:55");
}

TEST_CASE("hexdump-blank-sep")
{
    std::vector<std::byte> data{std::byte(0x11), std::byte(0x22), std::byte(0x33), std::byte(0x44)};
    CHECK(hexdump(data, "") == "11223344");
}

TEST_CASE("hexdump-long-sep")
{
    std::vector<std::byte> data{std::byte(0xAA), std::byte(0xBB), std::byte(0xCC)};
    CHECK(hexdump(data, ")*=*(") == "AA)*=*(BB)*=*(CC");
}
