#include "data/BarStream.h"
#include "data/ClusterCsvReader.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>
namespace spartak::data {
BarStream::BarStream(const SyntheticParams& params)
    : mode_(StreamMode::Synthetic),
      params_(params),
      price_(params.start_price),
      rng_(params.seed)
{
    if (params_.point <= 0.0)
        throw std::invalid_argument("BarStream: point must be > 0");
    if (params_.max_bars == 0)
        throw std::invalid_argument("BarStream: max_bars must be > 0");
    ok_ = true;
}
BarStream::BarStream(std::vector<core::Bar> bars)
    : mode_(StreamMode::Vector), rng_(0)
{
    source_ = std::move(bars);
    ok_ = true;
}
BarStream::BarStream(const std::string& csv_path)
    : mode_(StreamMode::File), rng_(0)
{
    ClusterCsvReader reader;
    if (!reader.open(csv_path)) {
        throw std::runtime_error("BarStream: cannot open CSV: " + csv_path);
    }
    core::Bar b;
    while (reader.read_next(b)) {
        source_.push_back(b);
    }
    reader.close();
    ok_ = true;
}
void BarStream::push_back(const core::Bar& bar) {
    replay_.push_back(bar);
}
bool BarStream::next(core::Bar& out) {
    if (!ok_) return false;
    if (!replay_.empty()) {
        out = replay_.front();
        replay_.pop_front();
        ++emitted_;
        return true;
    }
    if (mode_ == StreamMode::Synthetic) return next_synthetic(out);
    if (src_pos_ >= source_.size()) return false;
    out = source_[src_pos_++];
    ++emitted_;
    ++generated_;
    return true;
}
bool BarStream::next_synthetic(core::Bar& out) {
    if (generated_ >= static_cast<int64_t>(params_.max_bars)) return false;
    out = make_synthetic_bar();
    ++generated_;
    ++emitted_;
    return true;
}
core::Bar BarStream::make_synthetic_bar() {
    const double wave   = std::sin(static_cast<double>(generated_) / 12.0);
    std::uniform_real_distribution<double> noise(-2.5, 2.5);
    const double drift  = (wave * 15.0 + noise(rng_)) * params_.point;
    const double open   = price_;
    const double close  = price_ + drift;
    const double hi_off = (std::fabs(noise(rng_)) + 2.0) * params_.point;
    const double lo_off = (std::fabs(noise(rng_)) + 2.0) * params_.point;
    const double high   = std::max(open, close) + hi_off;
    const double low    = std::min(open, close) - lo_off;
    price_ = close;
    core::Bar b;
    b.timestamp   = 1'700'000'000'000LL
                  + static_cast<int64_t>(generated_) * 300'000LL;
    b.open        = open;
    b.high        = high;
    b.low         = low;
    b.close       = close;
    b.volume = 100;
    b.spread      = params_.spread_points;
    return b;
}
int64_t BarStream::total_bars() const noexcept {
    if (mode_ == StreamMode::Synthetic)
        return static_cast<int64_t>(params_.max_bars);
    return static_cast<int64_t>(source_.size());
}
} // namespace spartak::data