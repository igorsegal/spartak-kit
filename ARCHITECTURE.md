# SPARTAK KIT :: АРХИТЕКТУРА
Мастер-документ проекта. Обновляется при каждом изменении структуры.
Создан: 2026-10-02.
---
## 1. Паспорт
- **Название:** SPARTAK KIT
- **Назначение:** C++20 бэктестер торговой системы FOREX KIT.
- **Источник системы:** курс FOREX KIT (28 видео), docs/forex_kit_rules.md.
- **Источник данных:** ClusterDelta CSV (https://my.clusterdelta.com/files).
- **Стек:** C++20, CMake >= 3.20, MSVC 2022+.
- **Пространство имён:** spartak::core, spartak::data, spartak::cluster, ...
- **Репозиторий:** github.com/igorsegal/spartak-kit
---
## 2. Правила репозитория
### 2.1. Реестр файлов
Любой новый файл в `include/`, `src/`, `tests/` ДОЛЖЕН быть описан в разделе 4.
Новый файл без записи в реестре — CI красный.
### 2.2. Коммиты
- Один коммит — одно логическое изменение.
- Сообщение коммита: `[слой]: краткое описание`.
- Примеры: `data: add ClusterDelta CSV reader`, `cluster: add FootprintDetector`.
### 2.3. Ветки
- `main` — стабильная.
- Push в `main` только через PR.
### 2.4. Реестр без заглушек
Если в таблице реестра есть строки — фраза «Пока пусто» удаляется.
Заглушка и записи не сосуществуют.
### 2.5. Дисциплина статусов
Допустимо только пять значений:
- `plan`     — запланировано, работа не начата
- `WIP`      — идёт прямо сейчас
- `PASS`     — тесты прошли, зафиксировано
- `FAIL`     — тесты упали, откат обязателен
- `ROLLBACK` — откатили, шаг закрыт
Других значений нет. Статусы — идентификаторы. В файлах и в коде они
остаются на латинице. Описания — на кириллице.
### 2.6. Быстрый старт всегда актуален
Команды сборки и тестов живут в одном месте — раздел «Быстрый старт».
Меняешь команды — обновляешь блок в том же коммите.
### 2.7. Журнал синхронизируется постоянно
Всё, что попало в `JOURNAL.md`, немедленно копируется в зеркало.
Зеркало: `G:\Мой диск\AHexaTrader_BACKUP\spartak\`.
### 2.8. Push — только при изменении архитектуры
Промежуточные накопления журнала живут локально и в зеркале.
На GitHub идут только изменения `ARCHITECTURE.md`.
### 2.9. Одна запись — одно событие
Каждое событие — отдельная запись в журнале.
Если событие производное — оно идёт отдельной записью снизу.
Редактирование прошлых записей задним числом запрещено.
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
Реестр заполняется по мере закрытия шагов (PASS).
| Путь | Роль | Статус |
|---|---|---|
| `include/core/Types.h`         | Базовые типы (Bar, KitSignal)      | готов |
| `include/data/BarStream.h`     | Поток баров (Synthetic/Vector)     | готов |
| `src/data/BarStream.cpp`       | Реализация BarStream                | готов |
| `include/data/DataSanitizer.h` | Санитайзер (потоковый, BarStream)  | готов |
| `src/data/DataSanitizer.cpp`   | Реализация DataSanitizer            | готов |
| `include/data/ClusterCsvReader.h` | Парсер CSV ClusterDelta          | готов |
| `src/data/ClusterCsvReader.cpp`   | Реализация ClusterCsvReader      | готов |
| `tests/smoke_test.cpp`         | Smoke-тесты DataSanitizer/BarStream | готов |
| `tests/test_cluster_csv.cpp`   | Тест парсера CSV                    | готов |
| `tests/test_barstream_file.cpp`| Тест BarStream(File) + DataSanitizer | готов |
| `tests/csv_stats.cpp`          | Диагностика CSV (утилита)           | готов |
Статусы: `готов`, `в работе`, `заглушка`, `мусор`.
---
## 5. Поток данных
Пока не определён. Будет заполнен после создания слоя data/.
Планируемая схема:
    ClusterDelta CSV
      -> data::ClusterCsvReader
      -> data::BarStream (расширенный: delta, ask, bid)
      -> cluster::* (детекторы FOREX KIT)
      -> engine::BacktestPlayer
      -> BacktestReport
---
## 6. Стратегия FOREX KIT
Правила системы в отдельном документе: `docs/forex_kit_rules.md`.
Краткая сводка сигналов:
- Стоп-лосы покупателей и продавцов.
- Распродажи.
- Боковики (>= 4 касания).
- V-уровни, зеркальные уровни, плиты.
- Крупный игрок (парные цифры).
- Дельта, тотал-дельта.
---
## 7. Оптимизация
Пока не определена. Параметры для оптимизации:
- Размер парных цифр крупного игрока.
- Размер области стопов.
- Порог дельты.
---
## 8. Журнал результатов
Схема пока не определена. Минимум полей:
`instrument, period, signal_type, direction, entry, exit, pnl, comment`.
---
## 9. CI
Планируется GitHub Actions:
- Сборка на каждый push.
- Smoke-тесты.
- Проверка реестра (новый файл без записи — fail).
---
## 10. TODO
- [ ] Создать docs/forex_kit_rules.md — полные правила системы.
- [ ] Создать include/core/Types.h — базовые типы.
- [ ] Создать CMakeLists.txt.
- [ ] Реализовать data::ClusterCsvReader.
- [ ] Реализовать детекторы сигналов.
- [ ] Настроить CI.
---
## 11. Roadmap детекторов
Порядок — по §9 «Торговый процесс» и §7 «Многофакторный анализ»
из docs/forex_kit_rules.md.
### Доступно на текущих данных (bar-level ClusterDelta CSV)
OHLC + DELTA + общие ASK/BID на бар.
| # | Детектор           | Формация                          | Режим     | Шаг |
|---|--------------------|-----------------------------------|-----------|-----|
| 1 | RangeDetector      | Боковик >= 4 касания (§6.1)       | окно      | 007 |
| 2 | VLevelDetector     | V-уровень, импульс 2-9 свечей (§6.3) | окно  | 008 |
| 3 | MirrorLevelDetector| Зеркальный уровень (§6.5)         | окно      | 009 |
| 4 | DeltaDetector      | Дельта / тотал-дельта (§7)        | окно      | 010 |
| 5 | DivergenceDetector | Дивергенция RSI (§9)              | окно      | 011 |
| 6 | FalseBreakoutDetector | Ложный пробой + закрепление (§6) | окно  | 012 |
| 7 | VolumeProfileFilter| Профиль объёма, фильтр (§3)       | окно      | 013 |
### Требует footprint-данных (price-level clusters)
Кластеры по каждому ценовому уровню внутри бара. В текущей выгрузке
ClusterDelta отсутствуют. Ждём источник.
| # | Детектор            | Формация                          |
|---|---------------------|-----------------------------------|
| 8 | LiquidationDetector | Распродажа (§3, §6.4)             |
| 9 | StopLossDetector    | Стоп-лосы покупателей/продавцов (§3, §6.2) |
| 10| SlabDetector        | Плита (§6.6)                      |
| 11| LargePlayerDetector | Крупный игрок, парные цифры (§4)  |
| 12| ShadowDetector      | Тень (§6.7)                       |
### Зависимость
- CompositeSignal (§7): минимум 2-3 детектора из группы 1-7 должны работать.
---
## 12. Источник данных
**Правило 12.1. Базовый источник — .bin XFBAR.**
Локальные бинарные файлы в корне:
    D:\AHexaTrader\1DataFiles\raw\<SYMBOL>\<SYMBOL>_<TF>.bin
Например: D:\AHexaTrader\1DataFiles\raw\EURUSD\EURUSD_M5.bin
**Правило 12.2. Формат .bin.**
Структура заголовка и свечи — в docs/format_bin.md.
Изменение структуры — новая версия в поле magic. Без новой версии — не менять.
**Правило 12.3. Дополнительный источник — ClusterDelta CSV.**
Используется только для DELTA / ASK / BID. Не заменяет .bin и не
считается базовым.
**Правило 12.4. Сетевые источники запрещены.**
В бэктесте запрещены HTTP / API / любые online-источники.
Данные должны быть на диске до старта прогонки.
---
## 13. Терминология (Glossary)
Все термины торговой системы описаны в docs/glossary/.
Шаблон, статусы и правила ведения — в docs/glossary/README.md.
### Реестр файлов глоссария
| Файл | Содержание | Статус |
|---|---|---|
| `README.md` | Шаблон, статусы, правила ведения | утверждено |
| `01-fasy-rynka.md` | Накопление, тренд, распределение | утверждено |
| `02-struktura.md` | IT, ZO, ORT, RM, закрепление | утверждено |
| `03-signaly.md` | Дивергенция, ложный пробой, ловушка | утверждено |
| `04-urovni.md` | LU, PU, поддержка, сопротивление | утверждено |
| `05-obyom.md` | Добор, удержание, перелив | план |
| `06-formatcii.md` | Боковик, V-уровень, зеркальный, плита | план |
| `07-futprint.md` | Термины, ждущие footprint-данных | план |
### Статусы терминов
- `черновик` — записан, не проверен пользователем
- `утверждено` — проверен и утверждён
Термин из группы не переходит в `утверждено`, пока пользователь не подтвердил.
---
## 14. Протокол работы и зеркалирование
**Журнал:** `JOURNAL.md` — полная хронология шагов. Новые записи снизу.
**Источник правды:** репозиторий на GitHub.
**Зеркало:** `G:\Мой диск\AHexaTrader_BACKUP\spartak\` — копии
`JOURNAL.md` и `ARCHITECTURE.md`.
### Правило обновления этого файла
`ARCHITECTURE.md` обновляется **только после прохождения теста (PASS)**.
Промежуточные и неудачные шаги фиксируются исключительно в `JOURNAL.md`.
### Правило работы с источниками
Работаем только с `spartak-kit` и `spartak`.
Из «Спартака» переносим проверенные модули обработки данных, управления
позицией и движка. Логика ТАП не переносится.
### Синхронизация
    robocopy . "G:\Мой диск\AHexaTrader_BACKUP\spartak" JOURNAL.md ARCHITECTURE.md /XO
---
## 15. Реализованные модули (прошли smoke-тест)
### data/DataSanitizer  [шаг 002-004, PASS]
- Назначение: отсечение нерегулярного префикса потока баров.
- Отсекает Daily-префикс и гэпы. Стратегия стартует от первого регулярного бара.
- Файлы: `include/data/DataSanitizer.h`, `src/data/DataSanitizer.cpp`
- Источник: igorsegal/spartak (без изменения логики).
### data/BarStream  [шаг 003-004, PASS]
- Назначение: единая точка подачи баров в бэктест.
- Режимы: Synthetic (порт из «Спартака»), Vector (расширение spartak-kit).
- File-режим добавлен в шаге 006.
- Файлы: `include/data/BarStream.h`, `src/data/BarStream.cpp`
### tests/smoke_test  [шаг 003-004, PASS]
- 5 тестов DataSanitizer: пустой поток, регулярный поток,
  нерегулярный префикс, отсутствие регулярного хвоста, Synthetic-режим.
- Файл: `tests/smoke_test.cpp`
### data/ClusterCsvReader  [шаг 005, PASS]
- Назначение: парсинг CSV-выгрузок ClusterDelta (формат FOREX KIT).
- Формат: `OPEN_DATE;OPEN_TIME;OPEN;HIGH;LOW;CLOSE;VOLUME;DELTA;ASK;BID`.
- Особенности: дата DD.MM.YYYY, время HH:MM, timestamp в UTC,
  спред = 0 (в формате отсутствует), опция timezone_offset_hours.
- Файлы: `include/data/ClusterCsvReader.h`, `src/data/ClusterCsvReader.cpp`
### tests/test_cluster_csv  [шаг 005, PASS]
- Smoke-тест парсера на 5 барах из реальной выгрузки.
- Файл: `tests/test_cluster_csv.cpp`
- Данные: `tests/data/sample.csv`
### tests/csv_stats  [шаг 005c, PASS]
- Диагностическая утилита: статистика по любому CSV-файлу ClusterDelta.
- Печатает: кол-во баров, first/last ts, span, min low, max high,
  sum volume/delta/ask/bid.
- Файл: `tests/csv_stats.cpp`
### data/BarStream.FileMode  [шаг 006, PASS]
- Назначение: чтение CSV ClusterDelta напрямую через BarStream.
- Режим: StreamMode::File. Конструктор BarStream(const std::string& csv_path)
  читает весь файл через ClusterCsvReader в source_, далее работает
  ветка Vector (без дублирования кода в next()).
- Sanity: DataSanitizer.run() на 5-bar sample.csv — ok=false, bars_scanned=5
  (ожидаемо: confirm_bars=10, баров всего 5).
- Тесты: tests/test_barstream_file.cpp — 3/3 ctest прошли.
- Файлы: `include/data/BarStream.h`, `src/data/BarStream.cpp`
---
## Быстрый старт
    cmake -S . -B build
    cmake --build build --config Debug
    cd build
    ctest -C Debug --output-on-failure
    cd ..
Добавил новый файл в `include/` `src/` `tests/` — внеси его в раздел 4.
Добавил детектор — внеси в раздел 11.
**Обновлено:** 2026-10-03