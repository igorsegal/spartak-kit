// =============================================================================
//  spartak-kit :: data/DataSanitizer.h
//  Перенесено из igorsegal/spartak (data/DataSanitizer.h).
// =============================================================================
#pragma once
#include "core/Types.h"
#include "data/BarStream.h"
#include <cstdint>
namespace spartak::data {
struct SanitizeReport {
    bool    ok                      = false;
    int64_t bars_scanned            = 0;
    int64_t bars_skipped            = 0;
    int64_t first_regular_timestamp = 0;
    int64_t max_gap_seen_ms         = 0;
};
class DataSanitizer {
public:
    explicit DataSanitizer(int64_t max_gap_ms   = 14'400'000LL,
                           int     confirm_bars = 10);
    [[nodiscard]] SanitizeReport run(BarStream& stream) const;
    int64_t max_gap_ms()   const noexcept { return max_gap_ms_; }
    int     confirm_bars() const noexcept { return confirm_bars_; }
private:
    int64_t max_gap_ms_;
    int     confirm_bars_;
};
} // namespace spartak::data