#include "engine/BacktestPlayer.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace spartak::engine {

bool BacktestPlayer::validate() const {
    if (opts_.start_deposit <= 0.0) return false;
    if (opts_.risk_per_trade <= 0.0 || opts_.risk_per_trade >= 1.0) return false;
    if (opts_.stop_pct <= 0.0 || opts_.stop_pct >= 1.0) return false;
    if (opts_.profit_pct <= 0.0 || opts_.profit_pct >= 1.0) return false;
    if (opts_.window_size < 20) return false;
    if (opts_.signal_step < 1) return false;
    if (opts_.max_bars_in_trade < 1) return false;
    return true;
}

BacktestResult
BacktestPlayer::run(const std::vector<core::Bar>& bars) const {
    BacktestResult result;

    if (!validate()) return result;
    if (bars.size() < static_cast<std::size_t>(opts_.window_size) + 1) return result;

    cluster::CompositeSignal comp;

    double deposit = opts_.start_deposit;
    result.start_deposit = deposit;
    result.end_deposit   = deposit;

    double peak = deposit;
    double max_dd = 0.0;
    double max_dd_pct = 0.0;

    const std::size_t n = bars.size();
    const std::size_t win = static_cast<std::size_t>(opts_.window_size);
    const std::size_t step = static_cast<std::size_t>(opts_.signal_step);

    std::size_t i = win;
    while (i < n) {
        // Окно для CompositeSignal.
        std::vector<core::Bar> window(
            bars.begin() + static_cast<long>(i - win),
            bars.begin() + static_cast<long>(i));

        const auto sig = comp.find(window);
        if (!sig.ok) {
            i += step;
            continue;
        }

        // Открываем сделку по close бара i.
        Trade trade;
        trade.entry_ts      = bars[i].timestamp;
        trade.direction     = sig.direction;
        trade.entry_price   = bars[i].close;
        trade.bars_in_trade = 0;

        if (sig.direction == core::OrderSide::Buy) {
            trade.stop_price   = trade.entry_price * (1.0 - opts_.stop_pct);
            trade.profit_price = trade.entry_price * (1.0 + opts_.profit_pct);
        } else {
            trade.stop_price   = trade.entry_price * (1.0 + opts_.stop_pct);
            trade.profit_price = trade.entry_price * (1.0 - opts_.profit_pct);
        }

        // Ждём закрытия в последующих барах.
        bool closed = false;
        std::size_t max_j = std::min(i + 1 + static_cast<std::size_t>(opts_.max_bars_in_trade), n);

        for (std::size_t j = i + 1; j < max_j; ++j) {
            const auto& b = bars[j];

            if (sig.direction == core::OrderSide::Buy) {
                if (b.low <= trade.stop_price) {
                    trade.exit_price = trade.stop_price;
                    trade.reason     = TradeCloseReason::StopLoss;
                    closed = true;
                } else if (b.high >= trade.profit_price) {
                    trade.exit_price = trade.profit_price;
                    trade.reason     = TradeCloseReason::TakeProfit;
                    closed = true;
                }
            } else {
                if (b.high >= trade.stop_price) {
                    trade.exit_price = trade.stop_price;
                    trade.reason     = TradeCloseReason::StopLoss;
                    closed = true;
                } else if (b.low <= trade.profit_price) {
                    trade.exit_price = trade.profit_price;
                    trade.reason     = TradeCloseReason::TakeProfit;
                    closed = true;
                }
            }

            trade.exit_ts      = b.timestamp;
            trade.bars_in_trade = static_cast<int>(j - i);

            if (closed) break;
        }

        if (!closed) {
            trade.exit_price    = bars[max_j - 1].close;
            trade.exit_ts       = bars[max_j - 1].timestamp;
            trade.reason        = TradeCloseReason::MaxBars;
            trade.bars_in_trade = static_cast<int>(max_j - 1 - i);
        }

        // PnL считаем через проценты от текущего депозита.
        const double risk_amount = deposit * opts_.risk_per_trade;
        const double move_pct = std::fabs(trade.exit_price - trade.entry_price) / trade.entry_price;

        if (trade.reason == TradeCloseReason::TakeProfit) {
            const double rr = move_pct / opts_.stop_pct;
            trade.pnl = risk_amount * rr;
        } else if (trade.reason == TradeCloseReason::StopLoss) {
            trade.pnl = -risk_amount;
        } else {
            // MaxBars — считаем по факту движения.
            const bool win = (sig.direction == core::OrderSide::Buy)
                ? (trade.exit_price > trade.entry_price)
                : (trade.exit_price < trade.entry_price);
            trade.pnl = win ? (risk_amount * move_pct / opts_.stop_pct) : -risk_amount;
        }

        deposit += trade.pnl;
        if (deposit < 0.0) deposit = 0.0;

        if (trade.pnl > 0.0) ++result.wins;
        else ++result.losses;
        ++result.total_trades;

        if (deposit > peak) peak = deposit;
        const double dd = peak - deposit;
        const double dd_pct = (peak > 0.0) ? (dd / peak) * 100.0 : 0.0;
        if (dd > max_dd) max_dd = dd;
        if (dd_pct > max_dd_pct) max_dd_pct = dd_pct;

        result.trades.push_back(trade);

        // Продолжаем с шага после закрытия сделки.
        i = max_j;
    }

    result.end_deposit      = deposit;
    result.net_profit       = deposit - opts_.start_deposit;
    result.net_profit_pct   = (opts_.start_deposit > 0.0)
        ? (result.net_profit / opts_.start_deposit) * 100.0 : 0.0;
    result.win_rate_pct     = (result.total_trades > 0)
        ? (100.0 * result.wins / result.total_trades) : 0.0;
    result.max_drawdown     = max_dd;
    result.max_drawdown_pct = max_dd_pct;

    return result;
}

} // namespace spartak::engine