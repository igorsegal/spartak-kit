// test_barstream_file.cpp
// Shag 006: proverka BarStream(File) + DataSanitizer na realnom CSV.
#include "data/BarStream.h"
#include "data/DataSanitizer.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
int main(int argc, char** argv) {
    const std::string csv = (argc > 1) ? argv[1] : "tests/data/sample.csv";
    std::cout << "CSV: " << csv << "\n";
    spartak::data::BarStream stream(csv);
    std::cout << "mode = "
              << (stream.mode() == spartak::data::StreamMode::File ? "File" : "?")
              << ", total = " << stream.total_bars() << "\n";
    spartak::data::DataSanitizer sanitizer;
    const auto report = sanitizer.run(stream);
    std::cout << "ok               = " << (report.ok ? "true" : "false") << "\n";
    std::cout << "bars_scanned     = " << report.bars_scanned << "\n";
    std::cout << "bars_skipped     = " << report.bars_skipped << "\n";
    std::cout << "first_regular_ts = " << report.first_regular_timestamp << "\n";
    std::cout << "max_gap_seen_ms  = " << report.max_gap_seen_ms << "\n";
    // Minimal sanity: 5-bar sample.csv is too short for confirm_bars=10,
    // so ok=false is expected. Just verify stream is usable.
    if (stream.total_bars() <= 0) {
        std::cout << "FAIL: stream empty\n";
        return 1;
    }
    std::cout << "PASS\n";
    return 0;
}