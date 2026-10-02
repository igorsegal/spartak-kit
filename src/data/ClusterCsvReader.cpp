#include "data/ClusterCsvReader.h"
#include <stdexcept>
#include <string>
#include <vector>
namespace spartak::data {
namespace {
// Howard Hinnant, days_from_civil. Возвращает количество дней от 1970-01-01.
int64_t days_from_civil(int y, unsigned m, unsigned d) {
    y -= (m <= 2u);
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153u * (m + (m > 2u ? -3u : 9u)) + 2u) / 5u + d - 1u;
    const unsigned doe = yoe * 365u + yoe / 4u - yoe / 100u + doy;
    return static_cast<int64_t>(era) * 146097LL
         + static_cast<int64_t>(doe) - 719468LL;
}
bool parse_date(const std::string& s, int& y, unsigned& m, unsigned& d) {
    // DD.MM.YYYY
    if (s.size() < 10) return false;
    try {
        d = static_cast<unsigned>(std::stoi(s.substr(0, 2)));
        m = static_cast<unsigned>(std::stoi(s.substr(3, 2)));
        y = std::stoi(s.substr(6, 4));
    } catch (...) { return false; }
    return m >= 1u && m <= 12u && d >= 1u && d <= 31u;
}
bool parse_time(const std::string& s, int& h, int& mi) {
    // HH:MM
    if (s.size() < 5) return false;
    try {
        h  = std::stoi(s.substr(0, 2));
        mi = std::stoi(s.substr(3, 2));
    } catch (...) { return false; }
    return h >= 0 && h < 24 && mi >= 0 && mi < 60;
}
std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == delim) { out.push_back(cur); cur.clear(); }
        else cur.push_back(c);
    }
    out.push_back(cur);
    return out;
}
int64_t timestamp_ms(int y, unsigned m, unsigned d, int h, int mi, int tz_off) {
    const int64_t days = days_from_civil(y, m, d);
    const int64_t sec  = days * 86400LL
                       + static_cast<int64_t>(h) * 3600LL
                       + static_cast<int64_t>(mi) * 60LL
                       - static_cast<int64_t>(tz_off) * 3600LL;
    return sec * 1000LL;
}
} // namespace
ClusterCsvReader::ClusterCsvReader(CsvReaderOptions opts) : opts_(opts) {}
bool ClusterCsvReader::open(const std::string& path) {
    close();
    stream_.open(path, std::ios::in | std::ios::binary);
    if (!stream_.is_open()) return false;
    bars_read_ = 0;
    if (opts_.has_header) {
        std::string header;
        std::getline(stream_, header);
    }
    return true;
}
void ClusterCsvReader::close() {
    if (stream_.is_open()) stream_.close();
}
bool ClusterCsvReader::read_next(core::Bar& out) {
    if (!stream_.is_open()) return false;
    std::string line;
    while (std::getline(stream_, line)) {
        // убрать возможный \r в конце (CRLF)
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        const auto fields = split(line, opts_.delimiter);
        if (fields.size() < 10) continue; // пропустить мусорные строки
        int y; unsigned m, d; int h, mi;
        if (!parse_date(fields[0], y, m, d)) continue;
        if (!parse_time(fields[1], h, mi))  continue;
        try {
            out.timestamp   = timestamp_ms(y, m, d, h, mi, opts_.timezone_offset_hours);
            out.open        = std::stod(fields[2]);
            out.high        = std::stod(fields[3]);
            out.low         = std::stod(fields[4]);
            out.close       = std::stod(fields[5]);
            out.volume      = std::stoll(fields[6]);
            out.delta       = std::stoll(fields[7]);
            out.ask_volume  = std::stoll(fields[8]);
            out.bid_volume  = std::stoll(fields[9]);
            out.spread      = 0;
        } catch (...) {
            continue;
        }
        ++bars_read_;
        return true;
    }
    return false;
}
} // namespace spartak::data