#pragma once

#include "core/Types.h"
#include "cluster/CompositeSignal.h"
#include <cstdint>
#include <string>
#include <vector>

namespace spartak::engine {

struct BacktestOptions {
    double start_deposit      = 1000.0;
    double risk_per_trade     = 0.02;    // 2% депозита
    double stop_pct           = 0.005;   // 0.5% от цены входа
    double profit_pct         = 0.010;   // 1% от цены входа (соотношение 1:2)
    int    window_size        = 500;
    int    signal_step        = 100;     // проверять CompositeSignal раз в N баров
    int    max_bars_in_trade  = 200;     // принудительное закрытие через N баров
};

enum class TradeCloseReason {
    StopLoss,
    TakeProfit,
    MaxBars,
    EndOfData
};

struct Trade {
    std::int64_t    entry_ts        = 0;
    std::int64_t    exit_ts         = 0;
    core::OrderSide direction       = core::OrderSide::Buy;
    double          entry_price     = 0.0;
    double          exit_price      = 0.0;
    double          stop_price      = 0.0;
    double          profit_price    = 0.0;
    double          pnl             = 0.0;
    TradeCloseReason reason         = TradeCloseReason::EndOfData;
    int             bars_in_trade   = 0;
};

struct BacktestResult {
    double          start_deposit   = 0.0;
    double          end_deposit     = 0.0;
    double          net_profit      = 0.0;
    double          net_profit_pct  = 0.0;
    int             total_trades    = 0;
    int             wins            = 0;
    int             losses          = 0;
    double          win_rate_pct    = 0.0;
    double          max_drawdown    = 0.0;
    double          max_drawdown_pct = 0.0;
    std::vector<Trade> trades;
};

class BacktestPlayer {
public:
    BacktestPlayer() = default;
    explicit BacktestPlayer(BacktestOptions opts) : opts_(opts) {}

    void set_options(const BacktestOptions& opts) { opts_ = opts; }
    const BacktestOptions& options() const { return opts_; }

    bool validate() const;

    BacktestResult run(const std::vector<core::Bar>& bars) const;

private:
    BacktestOptions opts_{};
};

} // namespace spartak::engine