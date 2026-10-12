// csv_to_cbin.cpp
// Конвертер ClusterDelta CSV в бинарный формат CBIN.
// Использование: csv_to_cbin <input.csv> <output.cbin> <period_seconds> <digits> <point>

#include "data/ClusterCsvReader.h"
#include "data/CbinWriter.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace spartak;

int main(int argc, char** argv) {
    if (argc < 6) {
        std::cout << "usage: csv_to_cbin <input.csv> <output.cbin> <period_seconds> <digits> <point>\n";
        std::cout << "example: csv_to_cbin 6e_m5.csv 6e_m5.cbin 300 5 0.00001\n";
        return 1;
    }

    const std::string in_path = argv[1];
    const std::string out_path = argv[2];
    const int period_seconds = std::stoi(argv[3]);
    const int digits = std::stoi(argv[4]);
    const double point = std::stod(argv[5]);

    data::ClusterCsvReader reader;
    if (!reader.open(in_path)) {
        std::cout << "FAIL: cannot open " << in_path << "\n";
        return 1;
    }

    std::vector<core::Bar> bars;
    core::Bar bar;
    while (reader.read_next(bar)) {
        bars.push_back(bar);
    }
    reader.close();

    std::cout << "Read " << bars.size() << " bars from " << in_path << "\n";
    if (bars.empty()) {
        std::cout << "FAIL: no bars\n";
        return 1;
    }

    if (!data::CbinWriter::write(out_path, bars, period_seconds, digits, point)) {
        std::cout << "FAIL: cannot write " << out_path << "\n";
        return 1;
    }

    std::cout << "Wrote " << out_path << "\n";
    std::cout << "  bars: " << bars.size() << "\n";
    std::cout << "  first_ts: " << bars.front().timestamp << "\n";
    std::cout << "  last_ts: " << bars.back().timestamp << "\n";
    return 0;
}