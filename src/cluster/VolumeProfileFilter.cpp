#include "cluster/VolumeProfileFilter.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace spartak::cluster {

bool VolumeProfileFilter::validate() const {
    if (opts_.value_area_pct <= 0.0 || opts_.value_area_pct > 1.0) return false;
    if (opts_.min_bars_for_profile < 1) return false;
    if (opts_.neutral_threshold < 0.0 || opts_.neutral_threshold >= 1.0) return false;
    return true;
}

namespace {

bool is_finite_bar(const core::Bar& b) {
    if (!std::isfinite(b.open)) return false;
    if (!std::isfinite(b.high)) return false;
    if (!std::isfinite(b.low)) return false;
    if (!std::isfinite(b.close)) return false;
    if (!std::isfinite(b.volume)) return false;
    if (!std::isfinite(b.ask_volume)) return false;
    if (!std::isfinite(b.bid_volume)) return false;
    return true;
}

bool is_valid_bar(const core::Bar& b) {
    if (!is_finite_bar(b)) return false;
    if (b.high < b.low) return false;
    if (b.ask_volume < 0.0) return false;
    if (b.bid_volume < 0.0) return false;
    if (b.volume < 0.0) return false;
    if (b.volume > 0.0 && (b.ask_volume + b.bid_volume) == 0.0) return false;
    return true;
}

} // namespace

VolumeProfileSignal
VolumeProfileFilter::find(const std::vector<core::Bar>& bars) const {
    VolumeProfileSignal r;

    if (!validate()) return r;

    // Шаг 1. Единый проход — фильтрация.
    std::vector<core::Bar> valid;
    valid.reserve(bars.size());
    for (const auto& b : bars) {
        if (is_valid_bar(b)) valid.push_back(b);
    }

    // Шаг 2. Минимум валидных баров.
    const std::size_t n = valid.size();
    if (n < static_cast<std::size_t>(opts_.min_bars_for_profile)) return r;

    // Шаг 3. Диапазон.
    double min_low  = valid[0].low;
    double max_high = valid[0].high;
    for (const auto& b : valid) {
        if (b.low  < min_low)  min_low  = b.low;
        if (b.high > max_high) max_high = b.high;
    }
    if (max_high - min_low <= 0.0) return r;

    // Шаг 4. Число бинов.
    int bins = static_cast<int>(n / 2);
    if (bins < 10) bins = 10;
    if (bins > 50) bins = 50;

    // Шаг 5. Ширина бина.
    const double bin_width = (max_high - min_low) / static_cast<double>(bins);
    if (bin_width <= 0.0 || !std::isfinite(bin_width)) return r;

    // Шаги 6–7. Распределение объёмов по бинам.
    std::vector<double> ask_bins(static_cast<std::size_t>(bins), 0.0);
    std::vector<double> bid_bins(static_cast<std::size_t>(bins), 0.0);
    std::vector<double> vol_bins(static_cast<std::size_t>(bins), 0.0);

    for (const auto& b : valid) {
        // high == low: весь объём в бин close (с прижатием к диапазону).
        if (b.high == b.low) {
            double px = b.close;
            if (px < min_low) px = min_low;
            if (px > max_high) px = max_high;
            int idx = static_cast<int>((px - min_low) / bin_width);
            if (idx < 0) idx = 0;
            if (idx >= bins) idx = bins - 1;
            ask_bins[static_cast<std::size_t>(idx)] += b.ask_volume;
            bid_bins[static_cast<std::size_t>(idx)] += b.bid_volume;
            vol_bins[static_cast<std::size_t>(idx)] += b.ask_volume + b.bid_volume;
            continue;
        }

        // Обычный бар: распределяем по бинам, которые покрывает [low, high).
        const double bar_width = b.high - b.low;
        if (bar_width <= 0.0) continue;

        int first_bin = static_cast<int>((b.low - min_low) / bin_width);
        int last_bin  = static_cast<int>((b.high - min_low) / bin_width);
        if (first_bin < 0) first_bin = 0;
        if (last_bin >= bins) last_bin = bins - 1;
        if (first_bin > last_bin) continue;

        // Веса по бинам — доля пересечения [low, high) с бином.
        // Для простоты: равномерно по всем покрытым бинам, нормировка на bar_width.
        // Считаем точные доли.
        double total_weight = 0.0;
        std::vector<double> weights;
        weights.reserve(static_cast<std::size_t>(last_bin - first_bin + 1));
        for (int i = first_bin; i <= last_bin; ++i) {
            const double bin_low  = min_low + i * bin_width;
            const double bin_high = min_low + (i + 1) * bin_width;
            const double seg_low  = std::max(b.low,  bin_low);
            const double seg_high = std::min(b.high, bin_high);
            const double w = std::max(0.0, seg_high - seg_low);
            weights.push_back(w);
            total_weight += w;
        }
        if (total_weight <= 0.0) continue;

        for (std::size_t k = 0; k < weights.size(); ++k) {
            const int idx = first_bin + static_cast<int>(k);
            const double share = weights[k] / total_weight;
            ask_bins[static_cast<std::size_t>(idx)] += b.ask_volume * share;
            bid_bins[static_cast<std::size_t>(idx)] += b.bid_volume * share;
            vol_bins[static_cast<std::size_t>(idx)] += (b.ask_volume + b.bid_volume) * share;
        }
    }

    // Шаги 8–10. Суммы.
    double ask_total = 0.0;
    double bid_total = 0.0;
    int bins_used = 0;
    for (int i = 0; i < bins; ++i) {
        const double a = ask_bins[static_cast<std::size_t>(i)];
        const double bi = bid_bins[static_cast<std::size_t>(i)];
        ask_total += a;
        bid_total += bi;
        if (a + bi > 0.0) ++bins_used;
    }

    if (ask_total + bid_total == 0.0) return r;
    if (ask_total == 0.0 || bid_total == 0.0) return r;

    // Шаг 11. Imbalance.
    const double imbalance = (ask_total - bid_total) / (ask_total + bid_total);

    // Шаг 12. Нейтральность.
    if (std::fabs(imbalance) < opts_.neutral_threshold) return r;

    // Шаг 13. Направление.
    const core::OrderSide dir = (imbalance >= 0.0) ? core::OrderSide::Sell : core::OrderSide::Buy;

    // Шаг 14. POC.
    int poc_idx = 0;
    double poc_vol = vol_bins[0];
    for (int i = 1; i < bins; ++i) {
        if (vol_bins[static_cast<std::size_t>(i)] > poc_vol) {
            poc_vol = vol_bins[static_cast<std::size_t>(i)];
            poc_idx = i;
        }
    }
    const double poc_price = min_low + (poc_idx + 0.5) * bin_width;

    // Шаг 15. Value Area.
    const double total_vol = ask_total + bid_total;
    const double target = opts_.value_area_pct * total_vol;

    double va_low  = min_low + poc_idx * bin_width;
    double va_high = min_low + (poc_idx + 1) * bin_width;

    if (bins_used < 2) {
        va_low  = min_low;
        va_high = max_high;
    } else {
        double acc = vol_bins[static_cast<std::size_t>(poc_idx)];
        int up = poc_idx + 1;
        int dn = poc_idx - 1;
        while (acc < target && (up < bins || dn >= 0)) {
            const double v_up = (up < bins) ? vol_bins[static_cast<std::size_t>(up)] : -1.0;
            const double v_dn = (dn >= 0)    ? vol_bins[static_cast<std::size_t>(dn)] : -1.0;

            if (v_up < 0.0 && v_dn < 0.0) break;

            // При равенстве — сначала верхний.
            if (v_up >= v_dn) {
                acc += v_up;
                va_high = min_low + (up + 1) * bin_width;
                ++up;
            } else {
                acc += v_dn;
                va_low = min_low + dn * bin_width;
                --dn;
            }
        }
        if (acc < target) {
            // Не добрали — весь диапазон.
            va_low  = min_low;
            va_high = max_high;
        }
    }

    r.ok              = true;
    r.direction       = dir;
    r.poc             = poc_price;
    r.value_area_high = va_high;
    r.value_area_low  = va_low;
    r.ask_total       = ask_total;
    r.bid_total       = bid_total;
    r.imbalance       = imbalance;
    r.bins_used       = bins_used;
    r.total_volume    = total_vol;

    return r;
}

} // namespace spartak::cluster