// test_delta_volume_6e.cpp
#include "data/CbinReader.h"
#include "cluster/DeltaDetector.h"
#include "cluster/VolumeProfileFilter.h"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace spartak;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "usage: test_delta_volume_6e <path.cbin> [window_size]\n";
        return 1;
    }
    const std::string path = argv[1];
    const std::size_t win = (argc >= 3) ? static_cast<std::size_t>(std::stoi(argv[2])) : 500;

    std::vector<core::Bar> bars;
    if (!data::CbinReader::load(path, bars)) {
        std::cout << "FAIL: не удалось прочитать " << path << "\n";
        return 1;
    }
    std::cout << "Loaded " << bars.size() << " bars\n";
    if (bars.size() < win + 1) {
        std::cout << "Too few bars\n";
        return 1;
    }

    cluster::DeltaDetector delta;
    cluster::VolumeProfileFilter vprofile;

    long delta_calls = 0, delta_ok = 0, delta_buy = 0, delta_sell = 0;
    long vp_calls = 0, vp_ok = 0, vp_buy = 0, vp_sell = 0;

    const std::size_t step = win / 2;

    auto t0 = std::chrono::steady_clock::now();

    for (std::size_t start = 0; start + win <= bars.size(); start += step) {
        std::vector<core::Bar> window(
            bars.begin() + static_cast<long>(start),
            bars.begin() + static_cast<long>(start + win));

        {
            const auto r = delta.find(window);
            ++delta_calls;
            if (r.ok) {
                ++delta_ok;
                if (r.direction == core::OrderSide::Buy) ++delta_buy;
                else ++delta_sell;
            }
        }
        {
            const auto r = vprofile.find(window);
            ++vp_calls;
            if (r.ok) {
                ++vp_ok;
                if (r.direction == core::OrderSide::Buy) ++vp_buy;
                else ++vp_sell;
            }
        }
    }

    auto t1 = std::chrono::steady_clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    std::cout << "\n=== СТАТИСТИКА ===\n";
    std::cout << "DeltaDetector: calls=" << delta_calls << " ok=" << delta_ok
              << " (" << (100.0 * delta_ok / delta_calls) << "%)"
              << " buy=" << delta_buy << " sell=" << delta_sell << "\n";
    std::cout << "VolumeProfileFilter: calls=" << vp_calls << " ok=" << vp_ok
              << " (" << (100.0 * vp_ok / vp_calls) << "%)"
              << " buy=" << vp_buy << " sell=" << vp_sell << "\n";
    std::cout << "\nВремя прогона: " << ms << " мс\n";
    return 0;
}