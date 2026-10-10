#include "data/XfbarReader.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace spartak::data {

namespace {

#pragma pack(push, 1)
struct RawBar {
    std::int64_t time;
    double open;
    double high;
    double low;
    double close;
    std::int64_t tick_volume;
    std::int32_t spread;
    std::int64_t real_volume;
};
#pragma pack(pop)

bool read_header(std::FILE* f, XfbarHeader& h) {
    char magic[8];
    if (std::fread(magic, 1, 8, f) != 8) return false;
    h.magic.assign(magic, 8);

    if (std::fread(&h.version, sizeof(std::int32_t), 1, f) != 1) return false;
    if (std::fread(&h.record_size, sizeof(std::int32_t), 1, f) != 1) return false;
    if (std::fread(&h.period_seconds, sizeof(std::int32_t), 1, f) != 1) return false;
    if (std::fread(&h.digits, sizeof(std::int32_t), 1, f) != 1) return false;
    if (std::fread(&h.point, sizeof(double), 1, f) != 1) return false;
    if (std::fread(&h.bar_count, sizeof(std::int64_t), 1, f) != 1) return false;
    if (std::fread(&h.first_time, sizeof(std::int64_t), 1, f) != 1) return false;
    if (std::fread(&h.last_time, sizeof(std::int64_t), 1, f) != 1) return false;

    std::int32_t symbol_len = 0;
    if (std::fread(&symbol_len, sizeof(std::int32_t), 1, f) != 1) return false;
    if (symbol_len < 0 || symbol_len > 256) return false;

    if (symbol_len > 0) {
        std::string sym(static_cast<std::size_t>(symbol_len), '\0');
        if (std::fread(&sym[0], 1, static_cast<std::size_t>(symbol_len), f) !=
            static_cast<std::size_t>(symbol_len)) return false;
        h.symbol = sym;
    }

    return true;
}

void raw_to_bar(const RawBar& rb, core::Bar& b) {
    b.timestamp  = rb.time;
    b.open       = rb.open;
    b.high       = rb.high;
    b.low        = rb.low;
    b.close      = rb.close;
    b.volume     = static_cast<double>(rb.real_volume > 0 ? rb.real_volume : rb.tick_volume);
    b.ask_volume = b.volume * 0.5;
    b.bid_volume = b.volume * 0.5;
    b.delta      = 0.0;
}

} // namespace

bool XfbarReader::load(const std::string& path,
                       std::vector<core::Bar>& out,
                       XfbarHeader* header) {
    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return false;

    XfbarHeader h;
    if (!read_header(f, h)) { std::fclose(f); return false; }
    if (header) *header = h;

    if (h.bar_count <= 0) { std::fclose(f); return true; }

    out.clear();
    out.reserve(static_cast<std::size_t>(h.bar_count));

    RawBar rb;
    for (std::int64_t i = 0; i < h.bar_count; ++i) {
        if (std::fread(&rb, sizeof(RawBar), 1, f) != 1) break;
        core::Bar b;
        raw_to_bar(rb, b);
        out.push_back(b);
    }

    std::fclose(f);
    return true;
}

bool XfbarReader::load_head(const std::string& path,
                            std::vector<core::Bar>& out,
                            std::size_t max_bars,
                            XfbarHeader* header) {
    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return false;

    XfbarHeader h;
    if (!read_header(f, h)) { std::fclose(f); return false; }
    if (header) *header = h;

    if (h.bar_count <= 0 || max_bars == 0) { std::fclose(f); return true; }

    const std::int64_t n = std::min<std::int64_t>(
        h.bar_count, static_cast<std::int64_t>(max_bars));

    out.clear();
    out.reserve(static_cast<std::size_t>(n));

    RawBar rb;
    for (std::int64_t i = 0; i < n; ++i) {
        if (std::fread(&rb, sizeof(RawBar), 1, f) != 1) break;
        core::Bar b;
        raw_to_bar(rb, b);
        out.push_back(b);
    }

    std::fclose(f);
    return true;
}

bool XfbarReader::load_tail(const std::string& path,
                            std::vector<core::Bar>& out,
                            std::size_t max_bars,
                            XfbarHeader* header) {
    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return false;

    XfbarHeader h;
    if (!read_header(f, h)) { std::fclose(f); return false; }
    if (header) *header = h;

    if (h.bar_count <= 0 || max_bars == 0) { std::fclose(f); return true; }

    const std::int64_t skip = (h.bar_count > static_cast<std::int64_t>(max_bars))
        ? (h.bar_count - static_cast<std::int64_t>(max_bars))
        : 0;

    if (skip > 0) {
        const long offset = static_cast<long>(skip * static_cast<std::int64_t>(sizeof(RawBar)));
        if (std::fseek(f, offset, SEEK_CUR) != 0) { std::fclose(f); return false; }
    }

    const std::int64_t n = h.bar_count - skip;
    out.clear();
    out.reserve(static_cast<std::size_t>(n));

    RawBar rb;
    for (std::int64_t i = 0; i < n; ++i) {
        if (std::fread(&rb, sizeof(RawBar), 1, f) != 1) break;
        core::Bar b;
        raw_to_bar(rb, b);
        out.push_back(b);
    }

    std::fclose(f);
    return true;
}

} // namespace spartak::data