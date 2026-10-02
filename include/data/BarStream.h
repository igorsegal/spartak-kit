#pragma once
#include "core/Types.h"
#include <cstdint>
#include <cstddef>
#include <deque>
#include <random>
#include <string>
#include <vector>
namespace spartak::data {
enum class StreamMode {
    Synthetic,
    Vector,
    File
};
class BarStream {
public:
    struct SyntheticParams {
        double      start_price   = 1.10000;
        double      point         = 0.00001;
        int32_t     spread_points = 12;
        std::size_t max_bars      = 1000;
        uint32_t    seed          = 1337;
    };
    explicit BarStream(const SyntheticParams& params);
    explicit BarStream(std::vector<core::Bar> bars);
    explicit BarStream(const std::string& csv_path);
    BarStream(const BarStream&)            = delete;
    BarStream& operator=(const BarStream&) = delete;
    bool next(core::Bar& out);
    void push_back(const core::Bar& bar);
    StreamMode mode() const noexcept { return mode_; }
    int64_t    total_bars() const noexcept;
    int64_t    bars_read()  const noexcept { return emitted_; }
    bool       is_ok()      const noexcept { return ok_; }
private:
    StreamMode mode_     = StreamMode::Synthetic;
    bool       ok_       = true;
    int64_t    emitted_  = 0;
    int64_t    generated_ = 0;
    SyntheticParams params_{};
    double          price_ = 0.0;
    std::mt19937    rng_;
    std::vector<core::Bar> source_;
    std::size_t            src_pos_ = 0;
    std::deque<core::Bar> replay_;
    bool      next_synthetic(core::Bar& out);
    core::Bar make_synthetic_bar();
};
} // namespace spartak::data