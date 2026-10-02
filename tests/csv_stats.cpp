// =============================================================================
//  spartak-kit :: tests/csv_stats.cpp
//  Диагностическая утилита: парсит CSV-файл ClusterDelta и печатает статистику.
//  Работает с файлом любого размера.
//  Использование: csv_stats <path-to-csv>
// =============================================================================
#include "data/ClusterCsvReader.h"
#include <cstdint>
#include <iostream>
#include <string>
using namespace spartak;
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: csv_stats <path-to-csv>\n";
        return 2;
    }
    data::ClusterCsvReader reader;
    if (!reader.open(argv[1])) {
        std::cerr << "cannot open: " << argv[1] << "\n";
        return 2;
    }
    core::Bar b;
    int64_t n = 0;
    int64_t t_first = 0, t_last = 0;
    double  min_low = 0.0, max_high = 0.0;
    int64_t sum_vol = 0, sum_delta = 0;
    int64_t sum_ask = 0, sum_bid = 0;
    bool first = true;
    while (reader.read_next(b)) {
        if (first) {
            t_first  = b.timestamp;
            min_low  = b.low;
            max_high = b.high;
            first = false;
        }
        t_last = b.timestamp;
        if (b.low  < min_low)  min_low  = b.low;
        if (b.high > max_high) max_high = b.high;
        sum_vol   += b.volume;
        sum_delta += b.delta;
        sum_ask   += b.ask_volume;
        sum_bid   += b.bid_volume;
        ++n;
    }
    if (n == 0) {
        std::cerr << "no bars parsed\n";
        return 1;
    }
    std::cout << "File:        " << argv[1] << "\n";
    std::cout << "Bars:        " << n << "\n";
    std::cout << "First ts:    " << t_first << "\n";
    std::cout << "Last ts:     " << t_last  << "\n";
    std::cout << "Span (ms):   " << (t_last - t_first) << "\n";
    std::cout << "Min low:     " << min_low  << "\n";
    std::cout << "Max high:    " << max_high << "\n";
    std::cout << "Sum volume:  " << sum_vol  << "\n";
    std::cout << "Sum delta:   " << sum_delta << "\n";
    std::cout << "Sum ask:     " << sum_ask  << "\n";
    std::cout << "Sum bid:     " << sum_bid  << "\n";
    return 0;
}