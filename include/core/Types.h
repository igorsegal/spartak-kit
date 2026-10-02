// =============================================================================
//  SPARTAK KIT :: core/Types.h
//  Базовые типы для бэктестера FOREX KIT.
//
//  Отличие от SPARTAK:
//    - Bar расширен полями для кластеров (delta, ask, bid).
//    - Есть понятие ClusterLevel для работы с парными цифрами.
// =============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace spartak::core {
// -----------------------------------------------------------------------------
// 1. Рыночные данные
// -----------------------------------------------------------------------------
// Бар с кластерной информацией (формат ClusterDelta CSV).
struct Bar {
    int64_t timestamp   = 0;   // Unix ms
    double  open        = 0.0;
    double  high        = 0.0;
    double  low         = 0.0;
    double  close       = 0.0;
    int64_t volume      = 0;   // общий объём
    int64_t delta       = 0;   // дельта (ask - bid)
    int64_t ask_volume  = 0;   // объём по Ask (покупки)
    int64_t bid_volume  = 0;   // объём по Bid (продажи)
    int32_t spread      = 0;   // спред в пунктах (если есть)
};
// -----------------------------------------------------------------------------
// 2. Направление
// -----------------------------------------------------------------------------
enum class OrderSide {
    Buy,
    Sell
};
// -----------------------------------------------------------------------------
// 3. Типы сигналов FOREX KIT
// -----------------------------------------------------------------------------
enum class KitSignalType {
    None = 0,
    StopsBuyers,          // снятие стопов покупателей
    StopsSellers,         // снятие стопов продавцов
    Liquidation,          // распродажа
    Sideways,             // боковик
    VLevel,               // V-уровень
    MirrorLevel,          // зеркальный уровень
    Plate,                // плита
    Shadow,               // тень
    NewsLiquidity         // вход от ликвидности после новости
};
// -----------------------------------------------------------------------------
// 4. Определение крупного игрока
// -----------------------------------------------------------------------------
struct LargePlayer {
    bool     detected        = false;
    OrderSide side           = OrderSide::Buy;
    double   price_level     = 0.0;
    int      paired_digits   = 0;      // значение парных цифр (напр. 12 = 12-12)
    int64_t  volume          = 0;      // объём заявки
    int64_t  delta_at_level  = 0;      // значение дельты на уровне
    bool     hidden          = false;  // скрытый уровень
    // Для скрытого уровня: (contracts * 100000) / 2 -> убрать 3 нуля = delta_value
    int64_t  hidden_delta    = 0;
};
// -----------------------------------------------------------------------------
// 5. Готовый сигнал
// -----------------------------------------------------------------------------
struct KitSignal {
    bool          detected       = false;
    KitSignalType type           = KitSignalType::None;
    OrderSide     side           = OrderSide::Buy;
    double        trigger_price  = 0.0;
    double        level          = 0.0;
    double        zone_top       = 0.0;
    double        zone_bottom    = 0.0;
    double        suggested_stop = 0.0;
    double        target         = 0.0;
    double        confidence     = 0.0;  // 0..1, сколько факторов совпало
    int           factors_count  = 0;    // сколько факторов (>= 2 нужно)
};
} // namespace spartak::core