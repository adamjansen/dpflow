#include "dplib/util/hexdump.h"

#include <fmt/format.h>


std::string datapanel::util::hexdump(const std::vector<std::byte>& data) {
    auto out = fmt::memory_buffer();
    for (auto& b: data)
        fmt::format_to(std::back_inserter(out),"{:02X} ", b);

    return fmt::to_string(out);
}
