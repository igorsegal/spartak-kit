#pragma once

#include "core/Types.h"
#include <cstdint>
#include <string>
#include <vector>

namespace spartak::data {

struct CbinInfo {
    std::int32_t  version        = 0;
    std::int32_t  record_size    = 0;
    std::int32_t  period_seconds = 0;
    std::int32_t  digits         = 0;
    double        point          = 0.0;
    std::int64_t  bar_count      = 0;
    std::int64_t  first_time     = 0;
    std::int64_t  last_time      = 0;
};

class CbinReader {
public:
    static bool load(const std::string& path,
                     std::vector<core::Bar>& out,
                     CbinInfo* info = nullptr);
};

} // namespace spartak::data