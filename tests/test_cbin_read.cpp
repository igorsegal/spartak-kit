// test_cbin_read.cpp
#include "data/CbinReader.h"
#include <chrono>
#include <iostream>
#include <vector>
using namespace spartak;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "usage: test_cbin_read <path.cbin>\n";
        return 1;
    }

    auto t0 = std::chrono::steady_clock::now();

    data::CbinInfo info;
    std::vector<core::Bar> bars;
    if (!data::CbinReader::load(argv[1], bars, &info)) {
        std::cout << "FAIL: не удалось прочитать " << argv[1] << "\n";
        return 1;
    }

    auto t1 = std::chrono::steady_clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    std::cout << "CBIN loaded\n";
    std::cout << "  version=" << info.version << " record_size=" << info.record_size << "\n";
    std::cout << "  period_seconds=" << info.period_seconds << " digits=" << info.digits
              << " point=" << info.point << "\n";
    std::cout << "  bar_count=" << info.bar_count << "\n";
    std::cout << "  first_ts=" << info.first_time << " last_ts=" << info.last_time << "\n";
    std::cout << "  bars loaded=" << bars.size() << " за " << ms << " мс\n";

    if (!bars.empty()) {
        const auto& b = bars[0];
        std::cout << "  first bar: ts=" << b.timestamp
                  << " O=" << b.open << " H=" << b.high
                  << " L=" << b.low << " C=" << b.close
                  << " vol=" << b.volume << " delta=" << b.delta
                  << " ask=" << b.ask_volume << " bid=" << b.bid_volume << "\n";
    }
    return 0;
}