// test_backtest_6e.cpp
#include "data/CbinReader.h"
#include "engine/BacktestPlayer.h"
#include <chrono>
#include <iostream>
#include <vector>
using namespace spartak;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "usage: test_backtest_6e <path.cbin>\n";
        return 1;
    }

    std::vector<core::Bar> bars;
    if (!data::CbinReader::load(argv[1], bars)) {
        std::cout << "FAIL: не удалось прочитать " << argv[1] << "\n";
        return 1;
    }
    std::cout << "Loaded " << bars.size() << " bars\n";

    engine::BacktestOptions opts;
    opts.start_deposit     = 1000.0;
    opts.risk_per_trade    = 0.02;
    opts.stop_pct          = 0.005;
    opts.profit_pct        = 0.005;
    opts.window_size       = 500;
    opts.signal_step       = 100;
    opts.max_bars_in_trade = 500;

    engine::BacktestPlayer bt(opts);

    auto t0 = std::chrono::steady_clock::now();
    const auto result = bt.run(bars);
    auto t1 = std::chrono::steady_clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    std::cout << "\n=== РЕЗУЛЬТАТ ===\n";
    std::cout << "  start_deposit  = " << result.start_deposit << "\n";
    std::cout << "  end_deposit    = " << result.end_deposit << "\n";
    std::cout << "  net_profit     = " << result.net_profit
              << " (" << result.net_profit_pct << "%)\n";
    std::cout << "  total_trades   = " << result.total_trades << "\n";
    std::cout << "  wins           = " << result.wins << "\n";
    std::cout << "  losses         = " << result.losses << "\n";
    std::cout << "  win_rate       = " << result.win_rate_pct << "%\n";
    std::cout << "  max_drawdown   = " << result.max_drawdown
              << " (" << result.max_drawdown_pct << "%)\n";
    std::cout << "\nВремя: " << ms << " мс\n";

    if (!result.trades.empty()) {
        std::cout << "\nПоследние 5 сделок:\n";
        const int start = static_cast<int>(result.trades.size()) - 5;
        for (int i = start; i < static_cast<int>(result.trades.size()); ++i) {
            const auto& t = result.trades[i];
            std::cout << "  #" << i
                      << " " << (t.direction == core::OrderSide::Buy ? "BUY " : "SELL")
                      << " entry=" << t.entry_price
                      << " exit=" << t.exit_price
                      << " pnl=" << t.pnl
                      << " bars=" << t.bars_in_trade
                      << " reason=" << static_cast<int>(t.reason) << "\n";
        }
    }
    return 0;
}