#include "data/XfbarReader.h"
#include "cluster/CascadeLevelDetector.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <vector>
using namespace spartak;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "usage: debug_cascade <path-to-bin>\n";
        return 1;
    }
    std::vector<core::Bar> all;
    if (!data::XfbarReader::load(argv[1], all)) {
        std::cout << "load fail\n";
        return 1;
    }

    const std::size_t win = 500;
    std::vector<core::Bar> bars(all.begin(), all.begin() + static_cast<long>(win));

    double mn = bars[0].low, mx = bars[0].high;
    for (const auto& b : bars) {
        if (b.low < mn) mn = b.low;
        if (b.high > mx) mx = b.high;
    }
    const double range = mx - mn;
    const double tol = range * 0.01;
    const double last_close = bars.back().close;
    const double near_threshold = 0.4 * range;

    std::cout << "range=" << range << " tol=" << tol
              << " last_close=" << last_close
              << " near_threshold=" << near_threshold << "\n";

    // Кандидаты.
    std::vector<double> cands;
    for (const auto& b : bars) {
        if (b.high == b.low) continue;
        cands.push_back(b.high);
        cands.push_back(b.low);
    }

    // Уникализация.
    std::vector<double> levels;
    for (double lv : cands) {
        if (lv <= 0.0) continue;
        bool dup = false;
        for (double e : levels) if (std::fabs(e - lv) <= tol) { dup = true; break; }
        if (!dup) levels.push_back(lv);
    }

    // Касания и отбой.
    const int min_touches = 2;
    const int max_reb_bars = 50;
    const double min_reb_dist = 0.1 * range;

    struct Info { double level; std::size_t lt; std::size_t reb; int touches; };
    std::vector<Info> infos;

    for (double lv : levels) {
        std::vector<std::size_t> t;
        for (std::size_t i = 0; i < bars.size(); ++i)
            if (bars[i].low <= lv + tol && bars[i].high >= lv - tol) t.push_back(i);
        if (static_cast<int>(t.size()) < min_touches) continue;

        std::size_t lt = t.back();
        std::size_t end = std::min(lt + 1 + static_cast<std::size_t>(max_reb_bars), bars.size());
        std::size_t reb = 0;
        bool found = false;
        for (std::size_t i = lt + 1; i < end; ++i) {
            double c = bars[i].close;
            double ct = bars[lt].close;
            if (ct < lv) { if (lv - c >= min_reb_dist) { reb = i; found = true; break; } }
            else          { if (c - lv >= min_reb_dist) { reb = i; found = true; break; } }
        }
        if (!found) continue;
        infos.push_back({lv, lt, reb, static_cast<int>(t.size())});
    }

    std::cout << "infos (with rebound)=" << infos.size() << "\n";

    // Проверка пар.
    int near_ok = 0;
    int gap_ok = 0;
    for (std::size_t i = 0; i < infos.size(); ++i) {
        for (std::size_t j = i + 1; j < infos.size(); ++j) {
            if (std::fabs(infos[i].level - last_close) > near_threshold) continue;
            if (std::fabs(infos[j].level - last_close) > near_threshold) continue;
            ++near_ok;

            std::size_t ra = infos[i].reb, rb = infos[j].reb;
            std::size_t gap = (ra > rb) ? (ra - rb) : (rb - ra);
            if (gap > 200) continue;
            ++gap_ok;
        }
    }

    std::cout << "pairs near close=" << near_ok << "\n";
    std::cout << "pairs gap <= 200=" << gap_ok << "\n";

    cluster::CascadeLevelDetector d;
    const auto r = d.find(bars);
    std::cout << "find: ok=" << r.ok << "\n";
    return 0;
}