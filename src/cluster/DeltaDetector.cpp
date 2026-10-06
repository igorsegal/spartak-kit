#include "cluster/DeltaDetector.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace spartak::cluster {

bool DeltaDetector::validate() const {
    if (opts_.window_size < 2) return false;
    if (opts_.anomaly_k <= 0.0) return false;
    if (opts_.min_bars < 2) return false;
    if (opts_.min_bars > opts_.window_size) return false;
    if (opts_.direction_filter <= 0.0 || opts_.direction_filter > 1.0) return false;
    return true;
}

DeltaSignal
DeltaDetector::find(const std::vector<core::Bar>& bars) const {
    if (bars.empty()) return DeltaSignal{};
    return find_at(bars, bars.size() - 1);
}

DeltaSignal
DeltaDetector::find_last(const std::vector<core::Bar>& bars) const {
    return find(bars);
}

DeltaSignal
DeltaDetector::find_at(const std::vector<core::Bar>& bars, std::size_t index) const {
    DeltaSignal r;

    if (!validate()) return r;
    if (index >= bars.size()) return r;

    // Нужно min_bars баров до индекса для статистики.
    if (index + 1 < static_cast<std::size_t>(opts_.min_bars)) return r;

    // Окно: от index-window_size+1 до index включительно.
    const std::size_t win = static_cast<std::size_t>(opts_.window_size);
    const std::size_t start = (index + 1 > win) ? (index + 1 - win) : 0;
    const std::size_t count = index + 1 - start;

    // Среднее и stddev дельты по окну.
    double sum = 0.0;
    for (std::size_t i = start; i <= index; ++i) {
        sum += bars[i].delta;
    }
    const double avg = sum / static_cast<double>(count);

    double var_sum = 0.0;
    for (std::size_t i = start; i <= index; ++i) {
        const double d = bars[i].delta - avg;
        var_sum += d * d;
    }
    const double stddev = std::sqrt(var_sum / static_cast<double>(count));

    // Текущий бар.
    const auto& b = bars[index];
    const double delta = b.delta;
    const double abs_delta = std::fabs(delta);

    // Аномалия: |delta| > k * stddev.
    double anomaly_ratio = 0.0;
    if (stddev > 0.0) {
        anomaly_ratio = abs_delta / (opts_.anomaly_k * stddev);
    } else if (abs_delta > 0.0) {
        // Все значения одинаковы, но текущее отличается — аномалия.
        anomaly_ratio = 1.0;
    }

    if (anomaly_ratio < 1.0) return r;  // не аномалия

    // Направление: знак дельты.
    // Дельта положительная — преобладание ASK (покупки толпы).
    // Дельта отрицательная — преобладание BID (продажи толпы).
    const core::OrderSide dir = (delta > 0.0) ? core::OrderSide::Buy : core::OrderSide::Sell;

    // Контроль направления: доля от аномалии должна превысить фильтр.
    const double half_anomaly = opts_.anomaly_k * stddev * opts_.direction_filter;
    if (abs_delta < half_anomaly) return r;

    // Свеча: рост или падение.
    const double price_change = b.close - b.open;
    const bool candle_up = price_change > 0.0;

    // Противоречие: толпа покупает (delta > 0), а свеча падает.
    // Или толпа продаёт (delta < 0), а свеча растёт.
    bool contradiction = false;
    if (delta > 0.0 && price_change < 0.0) contradiction = true;
    if (delta < 0.0 && price_change > 0.0) contradiction = true;

    r.ok             = true;
    r.direction      = dir;
    r.timestamp      = b.timestamp;
    r.delta          = delta;
    r.delta_avg      = avg;
    r.delta_stddev   = stddev;
    r.anomaly_ratio  = anomaly_ratio;
    r.ask_vol        = b.ask_volume;
    r.bid_vol        = b.bid_volume;
    r.price_change   = price_change;
    r.candle_up      = candle_up;
    r.contradiction  = contradiction;

    return r;
}

} // namespace spartak::cluste