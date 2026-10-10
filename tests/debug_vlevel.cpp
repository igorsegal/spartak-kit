#include "data/XfbarReader.h"
#include "cluster/VLevelDetector.h"
#include <cmath>
#include <iostream>
#include <vector>
using namespace spartak;

int main(int argc, char** argv) {
    if (argc < 2) { std::cout << "usage: debug_vlevel <bin>\n"; return 1; }
    std::vector<core::Bar> all;
    if (!data::XfbarReader::load(argv[1], all)) return 1;

    const std::size_t win = 500;
    std::vector<core::Bar> bars(all.begin(), all.begin() + static_cast<long>(win));

    // Прогон с разными min_strength.
    const double strengths[] = {0.003, 0.005, 0.01, 0.02, 0.03, 0.05};
    for (double s : strengths) {
        cluster::VLevelOptions opts;
        opts.min_strength = s;
        cluster::VLevelDetector d(opts);
        const auto r = d.find(bars);
        std::cout << "min_strength=" << s
                  << " ok=" << r.ok
                  << " bars=" << r.bars_in_impulse
                  << " pct=" << r.impulse_pct << "\n";
    }

    // Прогон с разными one_way_ratio.
    const double ratios[] = {0.7, 0.8, 0.9, 1.0};
    for (double q : ratios) {
        cluster::VLevelOptions opts;
        opts.one_way_ratio = q;
        cluster::VLevelDetector d(opts);
        const auto r = d.find(bars);
        std::cout << "one_way_ratio=" << q
                  << " ok=" << r.ok
                  << " bars=" << r.bars_in_impulse << "\n";
    }

    return 0;
}