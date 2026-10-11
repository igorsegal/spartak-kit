#include "cluster/CompositeSignal.h"
#include "cluster/RangeDetector.h"
#include "cluster/VLevelDetector.h"
#include "cluster/MirrorLevelDetector.h"
#include "cluster/DivergenceDetector.h"
#include "cluster/FalseBreakoutDetector.h"
#include "cluster/CascadeLevelDetector.h"
#include "cluster/StopHuntDetector.h"
#include <cstddef>
#include <vector>

namespace spartak::cluster {

bool CompositeSignal::validate() const {
    if (opts_.min_weight < 1) return false;
    if (opts_.min_factors < 1) return false;
    if (opts_.min_lead < 0) return false;
    if (opts_.window_size < 20) return false;
    return true;
}

CompositeResult
CompositeSignal::find(const std::vector<core::Bar>& bars) const {
    CompositeResult r;

    if (!validate()) return r;
    if (bars.size() < 20) return r;

    // Обрезка окна.
    std::vector<core::Bar> window;
    if (bars.size() > static_cast<std::size_t>(opts_.window_size)) {
        window.assign(bars.end() - opts_.window_size, bars.end());
    } else {
        window = bars;
    }

    int buy_weight = 0;
    int sell_weight = 0;
    int buy_count = 0;
    int sell_count = 0;

    auto add_vote = [&](core::OrderSide dir, int weight) {
        if (dir == core::OrderSide::Buy) {
            buy_weight += weight;
            ++buy_count;
        } else {
            sell_weight += weight;
            ++sell_count;
        }
    };

        // Range — вес 3. Не даёт направления до пробоя.
    // Считается только как фактор (флаг), в голосовании не участвует.
    try {
        RangeDetector d;
        const auto s = d.find(window);
        if (s.ok) {
            r.range_fired = true;
            // Голос не добавляется — Range не определяет сторону.
        }
    } catch (...) {}

    // VLevel — вес 2.
    try {
        VLevelDetector d;
        const auto s = d.find(window);
        if (s.ok) {
            r.vlevel_fired = true;
            add_vote(s.direction, 2);
        }
    } catch (...) {}

    // Mirror — вес 3.
    try {
        MirrorLevelDetector d;
        const auto s = d.find(window);
        if (s.ok) {
            r.mirror_fired = true;
            add_vote(s.direction, 3);
        }
    } catch (...) {}

    // Divergence — вес 2.
    try {
        DivergenceDetector d;
        const auto s = d.find(window);
        if (s.ok) {
            r.divergence_fired = true;
            add_vote(s.direction, 2);
        }
    } catch (...) {}

    // FalseBreakout — вес 2.
    try {
        FalseBreakoutDetector d;
        const auto s = d.find(window);
        if (s.ok) {
            r.false_breakout_fired = true;
            add_vote(s.direction, 2);
        }
    } catch (...) {}

    // Cascade — вес 3.
    try {
        CascadeLevelDetector d;
        const auto s = d.find(window);
        if (s.ok) {
            r.cascade_fired = true;
            add_vote(s.direction, 3);
        }
    } catch (...) {}

    // StopHunt — вес 2.
    try {
        StopHuntDetector d;
        const auto s = d.find(window);
        if (s.ok) {
            r.stop_hunt_fired = true;
            add_vote(s.direction, 2);
        }
    } catch (...) {}

    // Победитель.
    if (buy_weight == sell_weight) {
        r.total_weight    = buy_weight;
        r.opposite_weight = sell_weight;
        r.factors_count   = buy_count > sell_count ? buy_count : sell_count;
        return r;  // ok = false
    }

    const bool buy_wins = buy_weight > sell_weight;
    const int winner_weight = buy_wins ? buy_weight : sell_weight;
    const int loser_weight  = buy_wins ? sell_weight : buy_weight;
    const int winner_count  = buy_wins ? buy_count : sell_count;

    r.total_weight    = winner_weight;
    r.opposite_weight = loser_weight;
    r.factors_count   = winner_count;
    r.direction       = buy_wins ? core::OrderSide::Buy : core::OrderSide::Sell;

    if (winner_weight < opts_.min_weight) return r;
    if (winner_count < opts_.min_factors) return r;
    if ((winner_weight - loser_weight) < opts_.min_lead) return r;

    r.ok = true;
    return r;
}

} // namespace spartak::cluster