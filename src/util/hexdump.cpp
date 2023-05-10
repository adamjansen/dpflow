#include "dplib/util/hexdump.h"

#include <fmt/format.h>

/**
 * @example hexdump.cpp
 *
 * This example demonstrates basic usage of hexdump.
 *
 * Output:
 * @code{.txt}
 * AA BB CC DD
 * AABBCCDD
 * AA : BB : CC : DD
 * @endcode
 */

std::string datapanel::util::hexdump(const std::vector<std::byte>& data, const std::string& sep) {
    auto out = fmt::memory_buffer();
    for (auto i=std::begin(data); i < std::end(data); ++i) {
        fmt::format_to(std::back_inserter(out),"{:02X}{}", *i, i == (std::end(data)-1) ? "": sep);
    }

    return fmt::to_string(out);
}
