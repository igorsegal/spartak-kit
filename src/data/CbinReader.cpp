#include "data/CbinReader.h"
#include "data/CbinFormat.h"
#include <cstdio>
#include <cstring>

namespace spartak::data {

bool CbinReader::load(const std::string& path,
                      std::vector<core::Bar>& out,
                      CbinInfo* info) {
    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return false;

    CbinHeader h;
    if (std::fread(&h, sizeof(h), 1, f) != 1) {
        std::fclose(f);
        return false;
    }

    if (std::memcmp(h.magic, "CBIN0001", 8) != 0) {
        std::fclose(f);
        return false;
    }

    if (info) {
        info->version        = h.version;
        info->record_size    = h.record_size;
        info->period_seconds = h.period_seconds;
        info->digits         = h.digits;
        info->point          = h.point;
        info->bar_count      = h.bar_count;
        info->first_time     = h.first_time;
        info->last_time      = h.last_time;
    }

    if (h.bar_count <= 0) {
        std::fclose(f);
        return true;
    }

    out.clear();
    out.reserve(static_cast<std::size_t>(h.bar_count));

    CbinBar rec;
    for (std::int64_t i = 0; i < h.bar_count; ++i) {
        if (std::fread(&rec, sizeof(rec), 1, f) != 1) break;
        core::Bar b;
        b.timestamp  = rec.time;
        b.open       = rec.open;
        b.high       = rec.high;
        b.low        = rec.low;
        b.close      = rec.close;
        b.volume     = static_cast<double>(rec.volume);
        b.delta      = static_cast<double>(rec.delta);
        b.ask_volume = static_cast<double>(rec.ask_volume);
        b.bid_volume = static_cast<double>(rec.bid_volume);
        out.push_back(b);
    }

    std::fclose(f);
    return true;
}

} // namespace spartak::data