#pragma once
#include "core/Types.h"
#include <cstddef>
#include <cstdint>
#include <vector>
namespace spartak::cluster {
// Сигнал V-уровня.
struct VLevelSignal {
    bool            ok               = false;
    core::OrderSide direction        = core::OrderSide::Buy;
    int64_t         start_ts         = 0;
    int64_t         end_ts           = 0;
    double          start_price      = 0.0;
    double          end_price        = 0.0;
    double          level_0          = 0.0;  // начало импульса
    double          level_50         = 0.0;  // середина
    double          level_100        = 0.0;  // конец импульса
    int             bars_in_impulse  = 0;
    double          impulse_pct      = 0.0;  // сила импульса в долях
    bool            is_broken        = false; // перебит ли импульс после
    bool            in_zone_50       = false; // цена сейчас у 50%
    bool            in_zone_100      = false; // цена сейчас у 100%
};
class VLevelDetector {
public:
    struct Options {
        std::size_t window_size       = 50;
        int         min_impulse_bars  = 2;
        int         max_impulse_bars  = 9;
        // Минимальная сила импульса — доля от средней цены.
        // 0.003 = 0.3%.
        double      min_strength      = 0.003;
        // Доля однонаправленных свечей в импульсе.
        // 0.8 = 80% свечей должны идти в сторону.
        double      one_way_ratio     = 0.8;
        // Допуск зоны входа у 50% или 100%.
        // 0.001 = 0.1%.
        double      zone_tolerance    = 0.001;
    };
    VLevelDetector() = default;
    explicit VLevelDetector(Options opts) : opts_(opts) {}
    [[nodiscard]] VLevelSignal find(const std::vector<core::Bar>& bars) const;
    [[nodiscard]] VLevelSignal find_last(const std::vector<core::Bar>& bars) const;
    [[nodiscard]] const Options& options() const noexcept { return opts_; }
private:
    Options opts_{};
};
} // namespace spartak::cluster