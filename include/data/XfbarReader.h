#pragma once

#include "core/Types.h"
#include <cstdint>
#include <string>
#include <vector>

namespace spartak::data {

struct XfbarHeader {
    std::string magic;          // "XFBAR001"
    std::int32_t version = 0;
    std::int32_t record_size = 0;
    std::int32_t period_seconds = 0;
    std::int32_t digits = 0;
    double point = 0.0;
    std::int64_t bar_count = 0;
    std::int64_t first_time = 0;
    std::int64_t last_time = 0;
    std::string symbol;
};

class XfbarReader {
public:
    // Загружает все бары из файла. Возвращает false при ошибке чтения.
    static bool load(const std::string& path,
                     std::vector<core::Bar>& out,
                     XfbarHeader* header = nullptr);

    // Загружает только первые max_bars баров.
    static bool load_head(const std::string& path,
                          std::vector<core::Bar>& out,
                          std::size_t max_bars,
                          XfbarHeader* header = nullptr);

    // Загружает только последние max_bars баров.
    static bool load_tail(const std::string& path,
                          std::vector<core::Bar>& out,
                          std::size_t max_bars,
                          XfbarHeader* header = nullptr);
};

} // namespace spartak::data