#include "data/CbinWriter.h"
#include "data/CbinFormat.h"
#include <cstdio>
#include <cstring>

namespace spartak::data {

bool CbinWriter::write(const std::string& path,
                       const std::vector<core::Bar>& bars,
                       int period_seconds,
                       int digits,
                       double point) {
    if (bars.empty()) return false;

    std::FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) return false;

    CbinHeader h;
    std::memset(&h, 0, sizeof(h));
    std::memcpy(h.magic, "CBIN0001", 8);
    h.version        = 1;
    h.record_size    = static_cast<std::int32_t>(sizeof(CbinBar));
    h.period_seconds = period_seconds;
    h.digits         = digits;
    h.point          = point;
    h.bar_count      = static_cast<std::int64_t>(bars.size());
    h.first_time     = bars.front().timestamp;
    h.last_time      = bars.back().timestamp;

    if (std::fwrite(&h, sizeof(h), 1, f) != 1) {
        std::fclose(f);
        return false;
    }

    for (const auto& b : bars) {
        CbinBar rec;
        rec.time       = b.timestamp;
        rec.open       = b.open;
        rec.high       = b.high;
        rec.low        = b.low;
        rec.close      = b.close;
        rec.volume     = static_cast<std::int64_t>(b.volume);
        rec.delta      = static_cast<std::int64_t>(b.delta);
        rec.ask_volume = static_cast<std::int64_t>(b.ask_volume);
        rec.bid_volume = static_cast<std::int64_t>(b.bid_volume);
        if (std::fwrite(&rec, sizeof(rec), 1, f) != 1) {
            std::fclose(f);
            return false;
        }
    }

    std::fclose(f);
    return true;
}

} // namespace spartak::data