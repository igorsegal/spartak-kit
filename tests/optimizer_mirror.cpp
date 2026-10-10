// optimizer_mirror.cpp
#include "data/XfbarReader.h"
#include "cluster/MirrorLevelDetector.h"
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
    int    min_touches;
    int    min_hold_bars;
    int    retest_window;
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
    std::cout << "] ";

    const int pct_int = static_cast<int>(pct * 100.0);
    std::cout << pct_int << "% ";
    std::cout << done << "/" << total << " ";

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
        std::cout << "usage: optimizer_mirror <bin> [--sample N] [--full] [--window W]\n";
        return 1;
    }

    const std::string path = argv[1];
    std::size_t sample_size = 30;
    std::size_t window = 500;
    bool full = false;

    for (int i = 2; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--full") full = true;
        else if (a == "--sample" && i + 1 < argc) sample_size = std::stoul(argv[++i]);
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
    if (full) {
        for (std::size_t s = 0; s <= max_start; s += window / 2) {
            starts.push_back(s);
        }
    } else {
        std::mt19937_64 rng(12345);
        std::uniform_int_distribution<std::size_t> dist(0, max_start);
        starts.reserve(sample_size);
        for (std::size_t i = 0; i < sample_size; ++i) {
            starts.push_back(dist(rng));
        }
    }

    std::cout << "Windows: " << starts.size() << ", window size: " << window << "\n";

    const std::vector<double> tol_grid     = {0.002, 0.005, 0.01, 0.02, 0.05};
    const std::vector<int>    touches_grid = {3, 4, 5};
    const std::vector<int>    hold_grid    = {5, 20, 50};
    const std::vector<int>    retest_grid  = {5, 20, 50};

    const std::size_t total_combos =
        tol_grid.size() * touches_grid.size() * hold_grid.size() * retest_grid.size();

    std::cout << "Combos: " << total_combos << "\n";
    std::cout << "Total find() calls: " << total_combos * starts.size() << "\n\n";

    std::vector<Result> results;
    results.reserve(total_combos);

    auto t0 = std::chrono::steady_clock::now();
    std::size_t combo_done = 0;

    for (double tol : tol_grid) {
        for (int mt : touches_grid) {
            for (int mh : hold_grid) {
                for (int rw : retest_grid) {
                    cluster::MirrorOptions opts;
                    opts.zone_tolerance_range = tol;
                    opts.min_touches          = mt;
                    opts.min_hold_bars        = mh;
                    opts.retest_window        = rw;
                    opts.window_size          = static_cast<int>(window);

                    cluster::MirrorLevelDetector d(opts);

                    Result r{tol, mt, mh, rw, 0, 0, 0, 0};
                    for (std::size_t s : starts) {
                        std::vector<core::Bar> win(
                            bars.begin() + static_cast<long>(s),
                            bars.begin() + static_cast<long>(s + window));
                        const auto sig = d.find(win);
                        ++r.calls;
                        if (sig.ok) {
                            ++r.ok;
                            if (sig.direction == core::OrderSide::Buy) ++r.buy;
                            else ++r.sell;
                        }
                    }
                    results.push_back(r);
                    ++combo_done;

                    auto now = std::chrono::steady_clock::now();
                    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - t0).count();
                    print_progress(combo_done, total_combos, ms);
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

    const std::string csv = "optimizer_mirror_results.csv";
    std::ofstream out(csv);
    out << "tol,min_touches,min_hold_bars,retest_window,ok_pct,ok,buy,sell,calls\n";
    for (const auto& r : results) {
        out << r.tol << ","
            << r.min_touches << ","
            << r.min_hold_bars << ","
            << r.retest_window << ","
            << ok_pct(r) << ","
            << r.ok << ","
            << r.buy << ","
            << r.sell << ","
            << r.calls << "\n";
    }
    out.close();

    std::cout << "\n=== ТОП-20 СЕЛЕКТИВНЫХ (низкий % сработок) ===\n";
    std::cout << "tol    mt  mh  rw   ok%      ok    buy   sell\n";
    for (std::size_t i = 0; i < std::min<std::size_t>(20, results.size()); ++i) {
        const auto& r = results[i];
        std::cout << r.tol << "  "
                  << r.min_touches << "   "
                  << r.min_hold_bars << "   "
                  << r.retest_window << "   "
                  << ok_pct(r) << "   "
                  << r.ok << "   "
                  << r.buy << "   "
                  << r.sell << "\n";
    }

    std::cout << "\n=== ТОП-20 ПЕРЕБОРНЫХ (высокий % сработок) ===\n";
    std::cout << "tol    mt  mh  rw   ok%      ok    buy   sell\n";
    for (std::size_t i = 0; i < std::min<std::size_t>(20, results.size()); ++i) {
        const auto& r = results[results.size() - 1 - i];
        std::cout << r.tol << "  "
                  << r.min_touches << "   "
                  << r.min_hold_bars << "   "
                  << r.retest_window << "   "
                  << ok_pct(r) << "   "
                  << r.ok << "   "
                  << r.buy << "   "
                  << r.sell << "\n";
    }

    std::cout << "\nВсего: " << total_ms << " мс. CSV: " << csv << "\n";
    return 0;
}