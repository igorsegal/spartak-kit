#pragma once

#include "core/Types.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace spartak::cluster {

struct DeltaSignal {
    bool            ok              = false;
    core::OrderSide direction       = core::OrderSide::Buy;
    std::int64_t    timestamp       = 0;
    double          delta           = 0.0;   // значение дельты на баре
    double          delta_avg       = 0.0;   // среднее в окне
    double          delta_stddev    = 0.0;   // стандартное отклонение в окне
    double          anomaly_ratio   = 0.0;   // |delta| / (k * stddev)
    double          ask_vol         = 0.0;
    double          bid_vol         = 0.0;
    double          price_change    = 0.0;   // close - open
    bool            candle_up       = false; // свеча роста
    bool            contradiction   = false; // толпа покупает, свеча падает (или наоборот)
};

struct DeltaOptions {
    int    window_size      = 50;
    double anomaly_k        = 2.5;
    int    min_bars         = 20;
    double direction_filter = 0.3;
};

class DeltaDetector {
public:
    DeltaDetector() = default;
    explicit DeltaDetector(DeltaOptions opts) : opts_(opts) {}

    void set_options(const DeltaOptions& opts) { opts_ = opts; }
    const DeltaOptions& options() const { return opts_; }

    bool validate() const;

    DeltaSignal find(const std::vector<core::Bar>& bars) const;
    DeltaSignal find_at(const std::vector<core::Bar>& bars, std::size_t index) const;
    DeltaSignal find_last(const std::vector<core::Bar>& bars) const;

private:
    DeltaOptions opts_{};
};

} // namespace spartak::cluster