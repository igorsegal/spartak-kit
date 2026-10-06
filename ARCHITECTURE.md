# SPARTAK KIT :: АРХИТЕКТУРА
Мастер-документ проекта. Обновляется при каждом изменении структуры.
Создан: 2026-10-02. Обновлено: 2026-10-06 (после ревизии правил FOREX KIT).
---
## 1. Паспорт
- **Название:** SPARTAK KIT
- **Назначение:** C++20 бэктестер торговой системы FOREX KIT.
- **Источник системы:** курс FOREX KIT (28 видео + 34 скриншота), `docs/forex_kit_rules.md` (v16, ревизия завершена 2026-10-06).
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
| include/cluster/DeltaDetector.h | Детектор аномальной дельты | готов |
| src/cluster/DeltaDetector.cpp | Реализация DeltaDetector | готов |
| tests/test_delta_detector.cpp | Тест DeltaDetector | готов |
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
Правила: `docs/forex_kit_rules.md` v16.
Ключевые сигналы: боковик (≥4 касания, ровный), V-уровень (2–9 свечей, на объёме), снятие стопов (требует уровня и зеркального уровня), распродажа (закрыта цифрами сверху и снизу), зеркальный уровень, плита, тень, уровень крупного игрока, каскадный уровень.
---
## 7. Оптимизация
- Размер парных цифр крупного игрока.
- Размер области стопов.
- Порог дельты.
- Ширина боковика (для расчёта потенциала 1:2).
---
## 8. Журнал результатов
instrument, period, signal_type, direction, entry, exit, pnl, comment.
---
## 9. CI
Планируется GitHub Actions: сборка, smoke-тесты, проверка реестра.
---
## 10. TODO
- [ ] Настроить CI.
- [ ] Применить правила к детекторам (после ревизии правил).
- [ ] Реализовать DeltaDetector.
- [ ] Реализовать MirrorLevelDetector.
- [ ] Реализовать StopHuntDetector.
---
## 11. Roadmap детекторов
### Доступно на текущих данных (bar-level ClusterDelta CSV)
| # | Детектор | Формация | Шаг |
|---|----------|----------|-----|
| 1 | RangeDetector | Боковик ≥ 4 касания, ровный, не перебитый | 007 PASS |
| 2 | VLevelDetector | V-уровень: импульс 2–9 свечей, виден на объёме | 016-017 PASS |
| 3 | MirrorLevelDetector | Зеркальный уровень: держали → пробили → ретест | 018 |
| 4 | DeltaDetector | Дельта / тотал-дельта: показывает толпу, след крупного игрока | 019 PASS |
| 5 | DivergenceDetector | Дивергенция RSI | 020 |
| 6 | FalseBreakoutDetector | Ложный пробой | 021 |
| 7 | VolumeProfileFilter | Профиль BitAsk: покупатели/продавцы, натягивать от начала боковика до пробития | 022 |
| 8 | CascadeLevelDetector | Каскадный уровень: два уровня подряд, два отбоя | 023 |
| 9 | StopHuntDetector | Снятие стопов: требует уровня и зеркального уровня, направление вниз | 024 |
### Требует footprint-данных (price-level clusters)
| # | Детектор |
|---|----------|
| 10 | LiquidationDetector |
| 11 | StopLossDetector |
| 12 | SlabDetector |
| 13 | LargePlayerDetector |
| 14 | ShadowDetector |
### Зависимость
CompositeSignal: минимум 2–3 фактора из группы 1–9.
---
## 12. Источник данных
**12.1.** Базовый — .bin XFBAR: `D:\AHexaTrader\1DataFiles\raw\<SYMBOL>\<SYMBOL>_<TF>.bin`.
**12.2.** Формат .bin — `docs/format_bin.md`.
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
**Синхронизация:** `robocopy . "G:\Мой диск\AHexaTrader_BACKUP\spartak" JOURNAL.md ARCHITECTURE.md /XO`.
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
Детектор боковика (≥ 4 касания границ).
### cluster/VLevelDetector [016-017 PASS]
Детектор V-уровня: импульс 2-9 свечей, уровни Фибо 0/50/100%.
Оптимизация: префиксные суммы, O(N·max_len).
Валидация: validate(). Работа с отрицательными ценами.
### Ревизия правил FOREX KIT [шаги 018-030 PASS]
- **Файл:** docs/forex_kit_rules.md v16.
- **Основание:** аудит 28 транскриптов + OCR 34 скриншотов.
- **Инструменты:** MCP-shell с командами `ask` и `synthesize`, DeepSeek API, Tesseract 5.5.3.
- **Закрыто 17 открытых вопросов.** Все ключевые термины и правила верифицированы.
- **Удалено:** «запертый объём» — термин не существует.
- **Готово к реализации детекторов.**
### cluster/DeltaDetector [019 PASS]
- **Назначение:** аномальная дельта, направление толпы, противоречие с ценой.
- **Алгоритм:** среднее и stddev дельты по окну 50 баров. Аномалия: |delta| > 2.5 × stddev. Направление — по знаку дельты. Противоречие: толпа покупает, свеча падает (или наоборот).
- **Файлы:** `include/cluster/DeltaDetector.h`, `src/cluster/DeltaDetector.cpp`.
- **Тест:** `tests/test_delta_detector.cpp` — 15/15 ctest.
- **analyze:** OK, дефектов не найдено.
---
## 16. Источники
- **Курс FOREX KIT:** 28 транскриптов в `forexkit_src/transcripts/`.
- **Скриншоты:** 34 изображения в `forexkit_src/SCREENS/`.
- **OCR скриншотов:** `SCREENS_OCR.txt` (генерируется Tesseract).
- **Правила системы:** `docs/forex_kit_rules.md` v16.
- **MCP-shell:** `D:\mcp-experiments\mcp_shell.py`.
---
## Быстрый старт
    cmake -S . -B build
    cmake --build build --config Debug
    cd build
    ctest -C Debug --output-on-failure
    cd ..
Новый файл в include/ src/ tests/ — в раздел 4.
Новый детектор — в раздел 11.