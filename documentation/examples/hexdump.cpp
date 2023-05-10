#include "dplib/util/hexdump.h"

#include <iostream>
#include <vector>

using datapanel::util::hexdump;

int main(int argc, char **argv) {

    std::vector<std::byte> data{
        std::byte(0xAA),
        std::byte(0xBB),
        std::byte(0xCC),
        std::byte(0xDD),
    };

    std::cout << hexdump(data) << std::endl;
    std::cout << hexdump(data, "") << std::endl;
    std::cout << hexdump(data, " : ") << std::endl;

    return 0;
}
