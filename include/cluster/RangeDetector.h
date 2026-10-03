#pragma once
#include "core/Types.h"
#include <cstddef>
#include <cstdint>
#include <vector>
namespace spartak::cluster {
// Сигнал боковика.
struct RangeSignal {
    bool            ok                = false;
    int64_t         start_ts          = 0;
    int64_t         end_ts            = 0;
    double          high              = 0.0;
    double          low               = 0.0;
    double          width             = 0.0;
    double          potential         = 0.0;
    int             touches_top       = 0;
    int             touches_bottom    = 0;
    int64_t         ask_sum           = 0;
    int64_t         bid_sum           = 0;
    core::OrderSide expected_breakout = core::OrderSide::Buy;
};
class RangeDetector {
public:
    struct Options {
        std::size_t window_size         = 50;
        // Допуск касания — доля, не процент.
        // 0.0015 = 0.15% от hi или lo.
        double      touch_tolerance     = 0.0015;
        int         min_touches_top     = 2;
        int         min_touches_bottom  = 2;
        // Минимальная ширина боковика — доля от средней цены.
        // 0.001 = 0.1%.
        double      min_width           = 0.001;
    };
    RangeDetector() = default;
    explicit RangeDetector(Options opts) : opts_(opts) {}
    [[nodiscard]] RangeSignal find(const std::vector<core::Bar>& bars) const;
    [[nodiscard]] RangeSignal find_last(const std::vector<core::Bar>& bars) const;
    [[nodiscard]] const Options& options() const noexcept { return opts_; }
private:
    Options opts_{};
};
} // namespace spartak::cluster