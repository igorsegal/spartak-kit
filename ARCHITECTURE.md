# SPARTAK KIT :: АРХИТЕКТУРА
Мастер-документ проекта. Обновляется при каждом изменении структуры.
Создан: 2026-10-02. Обновлено: 2026-10-04 (шаг [017] PASS).
---
## 1. Паспорт
- **Название:** SPARTAK KIT
- **Назначение:** C++20 бэктестер торговой системы FOREX KIT.
- **Источник системы:** курс FOREX KIT (28 видео), docs/forex_kit_rules.md.
- **Источник данных:** ClusterDelta CSV.
- **Стек:** C++20, CMake >= 3.20, MSVC 2022+.
- **Пространство имён:** spartak::core, spartak::data, spartak::cluster.
- **Репозиторий:** github.com/igorsegal/spartak-kit
---
## 2. Правила репозитория
### 2.1. Реестр файлов
Новый файл в include/, src/, tests/ — обязателен в разделе 4.
### 2.2. Коммиты
Формат: `[слой]: краткое описание`.
### 2.3. Ветки
main — стабильная. Push через PR.
### 2.4. Реестр без заглушек
### 2.5. Дисциплина статусов
plan | WIP | PASS | FAIL | ROLLBACK
### 2.6. Быстрый старт всегда актуален
### 2.7. Журнал синхронизируется постоянно
Зеркало: G:\Мой диск\AHexaTrader_BACKUP\spartak\.
### 2.8. Push — только при изменении архитектуры
### 2.9. Одна запись — одно событие
---
## 3. Планируемая структура
    include/core/        типы, константы, конфиг
    include/data/        чтение XFBAR и ClusterDelta CSV
    include/cluster/     детекторы сигналов FOREX KIT
    src/                 реализации
    tests/               smoke-тесты
    docs/                правила системы, ARCHITECTURE, ROADMAP
    tools/               конвертеры, CLI-утилиты
---
## 4. Реестр файлов
| Путь | Роль | Статус |
|---|---|---|
| include/core/Types.h | Базовые типы (Bar, KitSignal) | готов |
| include/data/BarStream.h | Поток баров (Synthetic/Vector/File) | готов |
| src/data/BarStream.cpp | Реализация BarStream | готов |
| include/data/DataSanitizer.h | Санитайзер | готов |
| src/data/DataSanitizer.cpp | Реализация DataSanitizer | готов |
| include/data/ClusterCsvReader.h | Парсер CSV ClusterDelta | готов |
| src/data/ClusterCsvReader.cpp | Реализация ClusterCsvReader | готов |
| include/cluster/RangeDetector.h | Детектор боковика | готов |
| src/cluster/RangeDetector.cpp | Реализация RangeDetector | готов |
| tests/test_range_detector.cpp | Тест RangeDetector | готов |
| include/cluster/VLevelDetector.h | Детектор V-уровня | готов |
| src/cluster/VLevelDetector.cpp | Реализация VLevelDetector | готов |
| tests/test_vlevel_detector.cpp | Тест VLevelDetector | готов |
| tests/smoke_test.cpp | Smoke-тесты DataSanitizer/BarStream | готов |
| tests/test_cluster_csv.cpp | Тест парсера CSV | готов |
| tests/test_barstream_file.cpp | Тест BarStream(File) + DataSanitizer | готов |
| tests/csv_stats.cpp | Диагностика CSV (утилита) | готов |
Статусы: готов, в работе, заглушка, мусор.
---
## 5. Поток данных
    ClusterDelta CSV
      -> data::ClusterCsvReader
      -> data::BarStream
      -> cluster::* (детекторы FOREX KIT)
      -> engine::BacktestPlayer
      -> BacktestReport
---
## 6. Стратегия FOREX KIT
Правила: docs/forex_kit_rules.md.
Сигналы: стоп-лосы покупателей/продавцов, распродажи, боковики (>=4 касания),
V-уровни, зеркальные уровни, плиты, крупный игрок (парные цифры), дельта, тотал-дельта.
---
## 7. Оптимизация
- Размер парных цифр крупного игрока.
- Размер области стопов.
- Порог дельты.
---
## 8. Журнал результатов
instrument, period, signal_type, direction, entry, exit, pnl, comment.
---
## 9. CI
Планируется GitHub Actions: сборка, smoke-тесты, проверка реестра.
---
## 10. TODO
- [ ] docs/forex_kit_rules.md
- [ ] Настроить CI
---
## 11. Roadmap детекторов
### Доступно на текущих данных
| # | Детектор | Формация | Шаг |
|---|----------|----------|-----|
| 1 | RangeDetector | Боковик >= 4 касания | 007 PASS |
| 2 | VLevelDetector | V-уровень, импульс 2-9 свечей | 016-017 PASS |
| 3 | MirrorLevelDetector | Зеркальный уровень | 018 |
| 4 | DeltaDetector | Дельта / тотал-дельта | 019 |
| 5 | DivergenceDetector | Дивергенция RSI | 020 |
| 6 | FalseBreakoutDetector | Ложный пробой | 021 |
| 7 | VolumeProfileFilter | Профиль объёма | 022 |
### Требует footprint-данных
| # | Детектор |
|---|----------|
| 8 | LiquidationDetector |
| 9 | StopLossDetector |
| 10 | SlabDetector |
| 11 | LargePlayerDetector |
| 12 | ShadowDetector |
---
## 12. Источник данных
**12.1.** Базовый — .bin XFBAR: D:\AHexaTrader\1DataFiles\raw\<SYMBOL>\<SYMBOL>_<TF>.bin
**12.2.** Формат .bin — docs/format_bin.md.
**12.3.** ClusterDelta CSV — только DELTA/ASK/BID.
**12.4.** Сетевые источники запрещены.
---
## 13. Терминология (Glossary)
| Файл | Содержание | Статус |
|---|---|---|
| README.md | Шаблон, статусы | утверждено |
| 01-fasy-rynka.md | Накопление, тренд, распределение | утверждено |
| 02-struktura.md | IT, ZO, ORT, RM, закрепление | утверждено |
| 03-signaly.md | Дивергенция, ложный пробой, ловушка | утверждено |
| 04-urovni.md | LU, PU, поддержка, сопротивление | утверждено |
| 05-obyom.md | Добор, удержание, перелив | план |
| 06-formatcii.md | Боковик, V-уровень, зеркальный, плита | план |
| 07-futprint.md | Footprint-термины | план |
---
## 14. Протокол работы и зеркалирование
**Журнал:** JOURNAL.md — хронология. Новые записи снизу.
**Источник правды:** GitHub.
**Зеркало:** G:\Мой диск\AHexaTrader_BACKUP\spartak\.
**Правило:** ARCHITECTURE обновляется только после PASS.
**Синхронизация:** robocopy . "G:\Мой диск\AHexaTrader_BACKUP\spartak" JOURNAL.md ARCHITECTURE.md /XO
---
## 15. Реализованные модули
### data/DataSanitizer [002-004 PASS]
Отсечение нерегулярного префикса потока баров.
### data/BarStream [003-004, 006 PASS]
Synthetic / Vector / File. File — через ClusterCsvReader.
### data/ClusterCsvReader [005 PASS]
Формат: OPEN_DATE;OPEN_TIME;OPEN;HIGH;LOW;CLOSE;VOLUME;DELTA;ASK;BID. Timestamp в UTC.
### tests/csv_stats [005c PASS]
Диагностика CSV: кол-во баров, first/last ts, span, min low, max high, sum volume/delta/ask/bid.
### cluster/RangeDetector [007 PASS]
Детектор боковика (>= 4 касания границ).
### cluster/VLevelDetector [016-017 PASS]
Детектор V-уровня: импульс 2-9 свечей, уровни Фибо 0/50/100%.
Оптимизация: префиксные суммы, O(N·max_len).
Валидация: validate(). Работа с отрицательными ценами.
---
## Быстрый старт
    cmake -S . -B build
    cmake --build build --config Debug
    cd build
    ctest -C Debug --output-on-failure
    cd ..
Новый файл в include/ src/ tests/ — в раздел 4.
Новый детектор — в раздел 11.