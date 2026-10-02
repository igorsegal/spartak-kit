// =============================================================================
//  spartak-kit :: data/DataSanitizer.cpp
//  Перенесено из igorsegal/spartak (data/DataSanitizer.cpp) без изменений логики.
// =============================================================================
#include "data/DataSanitizer.h"
#include <stdexcept>
#include <vector>
namespace spartak::data {
DataSanitizer::DataSanitizer(int64_t max_gap_ms, int confirm_bars)
    : max_gap_ms_(max_gap_ms), confirm_bars_(confirm_bars) {
    if (max_gap_ms_ <= 0)
        throw std::invalid_argument("DataSanitizer: max_gap_ms must be > 0");
    if (confirm_bars_ < 1)
        throw std::invalid_argument("DataSanitizer: confirm_bars must be >= 1");
}
SanitizeReport DataSanitizer::run(BarStream& stream) const {
    SanitizeReport rep;
    core::Bar prev;
    if (!stream.next(prev)) return rep;
    ++rep.bars_scanned;
    std::vector<core::Bar> regular;
    regular.reserve(confirm_bars_ + 1);
    int streak = 0;
    core::Bar cur;
    while (stream.next(cur)) {
        ++rep.bars_scanned;
        const int64_t delta = cur.timestamp - prev.timestamp;
        if (delta > rep.max_gap_seen_ms) rep.max_gap_seen_ms = delta;
        if (delta > 0 && delta <= max_gap_ms_) {
            if (streak == 0) {
                regular.clear();
                regular.push_back(prev);
                rep.first_regular_timestamp = prev.timestamp;
            }
            regular.push_back(cur);
            ++streak;
        } else {
            streak = 0;
            regular.clear();
            rep.first_regular_timestamp = 0;
        }
        if (streak >= confirm_bars_) {
            rep.ok = true;
            rep.bars_skipped = rep.bars_scanned - static_cast<int64_t>(regular.size());
            if (rep.bars_skipped < 0) rep.bars_skipped = 0;
            for (const auto& b : regular) stream.push_back(b);
            return rep;
        }
        prev = cur;
    }
    rep.ok = false;
    rep.bars_skipped = rep.bars_scanned;
    return rep;
}
} // namespace spartak::data