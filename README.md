# SPARTAK KIT
C++20 бэктестер торговой системы FOREX KIT.
## Назначение
Прогон кластерных данных (ClusterDelta CSV) через правила FOREX KIT:
- Определение крупного игрока (парные цифры, Ask/Bid).
- Стоп-лосы покупателей и продавцов.
- Распродажи (закрытые цифрами сверху/снизу).
- Боковики (>= 4 касания + профиль BitAsk).
- V-уровни, зеркальные уровни, плиты.
- Дельта и тотал-дельта.
## Статус
Проект на стадии инициализации. Источник системы — курс FOREX KIT (28 видео).
## Структура (планируемая)
    include/core/        типы, константы, конфиг
    include/data/        чтение XFBAR + ClusterDelta CSV
    include/cluster/     детекторы сигналов FOREX KIT
    src/                 реализации
    tests/               smoke-тесты
    docs/                ARCHITECTURE.md, ROADMAP.md, правила системы
## Сборка
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release