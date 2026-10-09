// crash_test.cpp
#include "data/XfbarReader.h"
#include "cluster/RangeDetector.h"
#include "cluster/VLevelDetector.h"
#include "cluster/MirrorLevelDetector.h"
#include "cluster/DeltaDetector.h"
#include "cluster/DivergenceDetector.h"
#include "cluster/FalseBreakoutDetector.h"
#include "cluster/VolumeProfileFilter.h"
#include "cluster/CascadeLevelDetector.h"
#include "cluster/StopHuntDetector.h"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
using namespace spartak;

namespace {

struct Stats {
    std::string name;
    long calls = 0;
    long ok = 0;
    long buy = 0;
    long sell = 0;
};

void print_stats(const Stats& s) {
    std::cout << s.name
              << ": calls=" << s.calls
              << " ok=" << s.ok
              << " buy=" << s.buy
              << " sell=" << s.sell
              << "\n";
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "usage: crash_test <path-to-bin> [window_size]\n";
        return 1;
    }
    const std::string path = argv[1];
    const std::size_t win = (argc >= 3)
        ? static_cast<std::size_t>(std::stoi(argv[2]))
        : 500;

    data::XfbarHeader h;
    std::vector<core::Bar> bars;
    std::cout << "Loading " << path << " ...\n";
    auto t0 = std::chrono::steady_clock::now();
    if (!data::XfbarReader::load(path, bars, &h)) {
        std::cout << "FAIL: не удалось загрузить файл\n";
        return 1;
    }
    auto t1 = std::chrono::steady_clock::now();
    const auto load_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    std::cout << "Header:\n";
    std::cout << "  magic=" << h.magic << " version=" << h.version
              << " record_size=" << h.record_size << "\n";
    std::cout << "  symbol=" << h.symbol
              << " period_seconds=" << h.period_seconds
              << " digits=" << h.digits
              << " point=" << h.point << "\n";
    std::cout << "  bars=" << h.bar_count
              << " first_ts=" << h.first_time
              << " last_ts=" << h.last_time << "\n";
    std::cout << "Loaded " << bars.size() << " bars за " << load_ms << " мс\n\n";

    if (bars.size() < win + 1) {
        std::cout << "Слишком мало баров: " << bars.size() << " < " << (win + 1) << "\n";
        return 1;
    }

    cluster::RangeDetector        range;
    cluster::VLevelDetector       vlevel;
    cluster::MirrorLevelDetector  mirror;
    cluster::DeltaDetector        delta;
    cluster::DivergenceDetector   diverg;
    cluster::FalseBreakoutDetector fbreak;
    cluster::VolumeProfileFilter  vprofile;
    cluster::CascadeLevelDetector cascade;
    cluster::StopHuntDetector     hunts;

    Stats st_range{"RangeDetector"};
    Stats st_vlevel{"VLevelDetector"};
    Stats st_mirror{"MirrorLevelDetector"};
    Stats st_delta{"DeltaDetector"};
    Stats st_diverg{"DivergenceDetector"};
    Stats st_fbreak{"FalseBreakoutDetector"};
    Stats st_vprofile{"VolumeProfileFilter"};
    Stats st_cascade{"CascadeLevelDetector"};
    Stats st_hunts{"StopHuntDetector"};

    const std::size_t step = win / 2;
    const std::size_t total_windows = (bars.size() - win) / step;

    std::cout << "Прогон: окно=" << win << ", шаг=" << step
              << ", окон=" << total_windows << "\n";

    auto t2 = std::chrono::steady_clock::now();

    for (std::size_t start = 0; start + win <= bars.size(); start += step) {
        std::vector<core::Bar> window(
            bars.begin() + static_cast<long>(start),
            bars.begin() + static_cast<long>(start + win));

        {
            const auto r = range.find(window);
            ++st_range.calls;
            if (r.ok) ++st_range.ok;
        }
        {
            const auto r = vlevel.find(window);
            ++st_vlevel.calls;
            if (r.ok) {
                ++st_vlevel.ok;
                if (r.direction == core::OrderSide::Buy) ++st_vlevel.buy;
                else ++st_vlevel.sell;
            }
        }
        {
            const auto r = mirror.find(window);
            ++st_mirror.calls;
            if (r.ok) {
                ++st_mirror.ok;
                if (r.direction == core::OrderSide::Buy) ++st_mirror.buy;
                else ++st_mirror.sell;
            }
        }
        {
            const auto r = delta.find(window);
            ++st_delta.calls;
            if (r.ok) {
                ++st_delta.ok;
                if (r.direction == core::OrderSide::Buy) ++st_delta.buy;
                else ++st_delta.sell;
            }
        }
        {
            const auto r = diverg.find(window);
            ++st_diverg.calls;
            if (r.ok) {
                ++st_diverg.ok;
                if (r.direction == core::OrderSide::Buy) ++st_diverg.buy;
                else ++st_diverg.sell;
            }
        }
        {
            const auto r = fbreak.find(window);
            ++st_fbreak.calls;
            if (r.ok) {
                ++st_fbreak.ok;
                if (r.direction == core::OrderSide::Buy) ++st_fbreak.buy;
                else ++st_fbreak.sell;
            }
        }
        {
            const auto r = vprofile.find(window);
            ++st_vprofile.calls;
            if (r.ok) {
                ++st_vprofile.ok;
                if (r.direction == core::OrderSide::Buy) ++st_vprofile.buy;
                else ++st_vprofile.sell;
            }
        }
        {
            const auto r = cascade.find(window);
            ++st_cascade.calls;
            if (r.ok) {
                ++st_cascade.ok;
                if (r.direction == core::OrderSide::Buy) ++st_cascade.buy;
                else ++st_cascade.sell;
            }
        }
        {
            const auto r = hunts.find(window);
            ++st_hunts.calls;
            if (r.ok) {
                ++st_hunts.ok;
                if (r.direction == core::OrderSide::Buy) ++st_hunts.buy;
                else ++st_hunts.sell;
            }
        }
    }

    auto t3 = std::chrono::steady_clock::now();
    const auto run_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();

    std::cout << "\n=== СТАТИСТИКА ===\n";
    print_stats(st_range);
    print_stats(st_vlevel);
    print_stats(st_mirror);
    print_stats(st_delta);
    print_stats(st_diverg);
    print_stats(st_fbreak);
    print_stats(st_vprofile);
    print_stats(st_cascade);
    print_stats(st_hunts);

    std::cout << "\nВремя прогона: " << run_ms << " мс\n";
    return 0;
}