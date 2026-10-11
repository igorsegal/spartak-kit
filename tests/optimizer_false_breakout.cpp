// optimizer_false_breakout.cpp
#include "data/XfbarReader.h"
#include "cluster/FalseBreakoutDetector.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace spartak;

namespace {

struct Result {
    double tol;
    int    mt;
    int    bb;
    int    mbr;
    long   ok;
    long   buy;
    long   sell;
    long   calls;
};

double ok_pct(const Result& r) {
    return r.calls > 0 ? 100.0 * static_cast<double>(r.ok) / r.calls : 0.0;
}

void print_progress(std::size_t done, std::size_t total, long long ms_elapsed) {
    const int width = 30;
    const double pct = (total > 0) ? static_cast<double>(done) / total : 0.0;
    const int filled = static_cast<int>(pct * width);
    std::cout << "\r[";
    for (int i = 0; i < width; ++i) {
        std::cout << ((i < filled) ? '#' : '-');
    }
    std::cout << "] " << static_cast<int>(pct * 100.0) << "% "
              << done << "/" << total << " ";
    long long eta_s = 0;
    if (done > 0) {
        eta_s = ms_elapsed * static_cast<long long>(total - done) / done / 1000;
    }
    std::cout << "ETA " << eta_s << "s   ";
    std::cout.flush();
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "usage: optimizer_false_breakout <bin> [--sample N] [--window W]\n";
        return 1;
    }

    const std::string path = argv[1];
    std::size_t sample_size = 100;
    std::size_t window = 300;

    for (int i = 2; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--sample" && i + 1 < argc) sample_size = std::stoul(argv[++i]);
        else if (a == "--window" && i + 1 < argc) window = std::stoul(argv[++i]);
    }

    std::vector<core::Bar> bars;
    if (!data::XfbarReader::load(path, bars)) {
        std::cout << "load fail\n";
        return 1;
    }
    std::cout << "Loaded " << bars.size() << " bars\n";

    if (bars.size() < window + 1) {
        std::cout << "Too few bars\n";
        return 1;
    }

    std::vector<std::size_t> starts;
    const std::size_t max_start = bars.size() - window;
    std::mt19937_64 rng(12345);
    std::uniform_int_distribution<std::size_t> dist(0, max_start);
    starts.reserve(sample_size);
    for (std::size_t i = 0; i < sample_size; ++i) {
        starts.push_back(dist(rng));
    }

    std::cout << "Windows: " << starts.size() << ", window size: " << window << "\n";

    const std::vector<double> tol_grid = {0.0005, 0.001, 0.002};
    const std::vector<int>    mt_grid  = {5, 8};
    const std::vector<int>    bb_grid  = {5, 10};
    const std::vector<int>    mbr_grid = {5, 10, 15};

    const std::size_t total =
        tol_grid.size() * mt_grid.size() * bb_grid.size() * mbr_grid.size();

    std::cout << "Combos: " << total << "\n";
    std::cout << "Total find() calls: " << total * starts.size() << "\n\n";

    std::vector<Result> results;
    results.reserve(total);

    auto t0 = std::chrono::steady_clock::now();
    std::size_t done = 0;

    for (double tol : tol_grid) {
        for (int mt : mt_grid) {
            for (int bb : bb_grid) {
                for (int mbr : mbr_grid) {
                    cluster::FalseBreakoutOptions opts;
                    opts.zone_tolerance_range  = tol;
                    opts.min_touches           = mt;
                    opts.breakout_min_bars     = bb;
                    opts.max_bars_since_return = mbr;
                    opts.window_size           = static_cast<int>(window);

                    cluster::FalseBreakoutDetector d(opts);

                    Result r{tol, mt, bb, mbr, 0, 0, 0, 0};
                    for (std::size_t s : starts) {
                        std::vector<core::Bar> w(
                            bars.begin() + static_cast<long>(s),
                            bars.begin() + static_cast<long>(s + window));
                        const auto sig = d.find(w);
                        ++r.calls;
                        if (sig.ok) {
                            ++r.ok;
                            if (sig.direction == core::OrderSide::Buy) ++r.buy;
                            else ++r.sell;
                        }
                    }
                    results.push_back(r);
                    ++done;

                    auto now = std::chrono::steady_clock::now();
                    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - t0).count();
                    print_progress(done, total, ms);
                }
            }
        }
    }
    std::cout << "\n";

    auto t1 = std::chrono::steady_clock::now();
    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    std::sort(results.begin(), results.end(),
              [](const Result& a, const Result& b) {
                  return ok_pct(a) < ok_pct(b);
              });

    const std::string csv = "optimizer_false_breakout_results.csv";
    std::ofstream out(csv);
    out << "tol,min_touches,breakout_min_bars,max_bars_since_return,ok_pct,ok,buy,sell,calls\n";
    for (const auto& r : results) {
        out << r.tol << "," << r.mt << "," << r.bb << "," << r.mbr << ","
            << ok_pct(r) << "," << r.ok << "," << r.buy << "," << r.sell << "," << r.calls << "\n";
    }
    out.close();

    std::cout << "\nВсего: " << total_ms << " мс. CSV: " << csv << "\n\n";
    std::cout << "tol    mt  bb  mbr   ok%      ok    buy   sell\n";
    for (const auto& r : results) {
        std::cout << r.tol << "  " << r.mt << "   " << r.bb << "   " << r.mbr << "   "
                  << ok_pct(r) << "   " << r.ok << "   " << r.buy << "   " << r.sell << "\n";
    }

    return 0;
}