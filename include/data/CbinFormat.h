#pragma once

#include <cstdint>

namespace spartak::data {

#pragma pack(push, 1)

// Заголовок файла CBIN.
// Размер: 64 байта.
struct CbinHeader {
    char          magic[8];          // "CBIN0001"
    std::int32_t  version;           // 1
    std::int32_t  record_size;       // 72
    std::int32_t  period_seconds;    // 300 для M5
    std::int32_t  digits;            // 5 для EURUSD
    double        point;             // 0.00001
    std::int64_t  bar_count;
    std::int64_t  first_time;
    std::int64_t  last_time;
    char          reserved[8];       // выравнивание до 64
};

// Запись одного бара.
// Размер: 72 байта.
struct CbinBar {
    std::int64_t  time;
    double        open;
    double        high;
    double        low;
    double        close;
    std::int64_t  volume;
    std::int64_t  delta;
    std::int64_t  ask_volume;
    std::int64_t  bid_volume;
};

#pragma pack(pop)

} // namespace spartak::data