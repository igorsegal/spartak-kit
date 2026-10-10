#include "data/XfbarReader.h"
#include "cluster/MirrorLevelDetector.h"
#include <iostream>
#include <vector>
using namespace spartak;

int main(int argc, char** argv) {
    if (argc < 2) { std::cout << "usage: debug_mirror_params <bin>\n"; return 1; }
    std::vector<core::Bar> all;
    if (!data::XfbarReader::load(argv[1], all)) return 1;

    const std::size_t win = 500;
    std::vector<core::Bar> bars(all.begin(), all.begin() + static_cast<long>(win));

        std::cout << "=== zone_tolerance_range ===\n";
    for (double zt : {0.05, 0.02, 0.01, 0.005, 0.002}) {
        cluster::MirrorOptions opts;
        opts.zone_tolerance_range = zt;
        cluster::MirrorLevelDetector d(opts);
        const auto r = d.find(bars);
        std::cout << "zone_tolerance_range=" << zt << " ok=" << r.ok
                  << " touches=" << r.touches_before_break
                  << " retest_ok=" << r.retest_ok << "\n";
    }

    std::cout << "=== min_hold_bars ===\n";
    for (int mh : {5, 20, 50, 100}) {
        cluster::MirrorOptions opts;
        opts.min_hold_bars = mh;
        cluster::MirrorLevelDetector d(opts);
        const auto r = d.find(bars);
        std::cout << "min_hold_bars=" << mh << " ok=" << r.ok << "\n";
    }

    std::cout << "=== retest_window ===\n";
    for (int rw : {5, 20, 50, 100}) {
        cluster::MirrorOptions opts;
        opts.retest_window = rw;
        cluster::MirrorLevelDetector d(opts);
        const auto r = d.find(bars);
        std::cout << "retest_window=" << rw << " ok=" << r.ok << "\n";
    }

    return 0;
}