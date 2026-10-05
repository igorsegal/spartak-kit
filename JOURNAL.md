# JOURNAL - spartak-kit
> Хронология всех шагов проекта. Новые записи добавляются **снизу**.
> Ничего не удаляется и не редактируется задним числом.
---
## [001] 2026-10-02 - Учреждение протокола работы
**Статус:** успех
**Автор:** igorsegal + assistant
**Затронуты файлы:** `JOURNAL.md` (создан), `ARCHITECTURE.md` (раздел «Протокол работы»)
**Коммит:** -
### Что сделано
Договорились о протоколе ведения проекта.
### Правила
1. Работаем только с `spartak-kit` и `spartak`.
2. Из «Спартака» переносим проверенные модули.
3. Пишем с нуля: ClusterCsvReader, Detectors, tests.
4. Каждый шаг — запись в журнале.
5. ARCHITECTURE — только PASS.
6. Новые записи — снизу.
7. Формат — PowerShell-скрипт.
8. Снимки не делаем.
9. Зеркало: G:\Мой диск\AHexaTrader_BACKUP\spartak\.
---
## [002] 2026-10-02 - Перенос DataSanitizer
**Статус:** успех
**Автор:** assistant
**Затронуты файлы:** include/data/DataSanitizer.h, src/data/DataSanitizer.cpp
### Что сделано
Перенесены DataSanitizer.h/.cpp из igorsegal/spartak без изменения логики.
---
## [003] 2026-10-02 - BarStream + smoke_test
**Статус:** успех
**Автор:** assistant
**Затронуты файлы:** include/data/BarStream.h, src/data/BarStream.cpp, tests/smoke_test.cpp, CMakeLists.txt
### Что сделано
Порт BarStream + Vector mode. DataSanitizer переписан. smoke_test — 5 тестов.
---
## [005] 2026-10-02 - ClusterCsvReader + smoke-test
**Статус:** PASS
**Автор:** assistant
**Затронуты файлы:** include/data/ClusterCsvReader.h, src/data/ClusterCsvReader.cpp, tests/test_cluster_csv.cpp, tests/data/sample.csv, CMakeLists.txt
### Что сделано
ClusterCsvReader под формат ClusterDelta: разделитель ;, колонки OPEN_DATE;OPEN_TIME;OPEN;HIGH;LOW;CLOSE;VOLUME;DELTA;ASK;BID. Timestamp в UTC. Checks: 15, Failures: 0.
---
## [005c] 2026-10-02 - Fix CMakeLists.txt + csv_stats
**Статус:** PASS
**Автор:** assistant
**Затронуты файлы:** CMakeLists.txt, tests/csv_stats.cpp
### Что сделано
Переписан CMakeLists.txt с нуля. csv_stats обработал реальный файл: 232389 баров.
---
## [006] 2026-10-02 - BarStream File mode
**Статус:** PASS
**Автор:** assistant
**Затронуты файлы:** include/data/BarStream.h, src/data/BarStream.cpp, tests/test_barstream_file.cpp
### Что сделано
BarStream(File) читает CSV через ClusterCsvReader. 3/3 ctest прошли.
---
## [007] 2026-10-02 - RangeDetector
**Статус:** PASS
**Автор:** assistant
**Затронуты файлы:** include/cluster/RangeDetector.h, src/cluster/RangeDetector.cpp, tests/test_range_detector.cpp
### Что сделано
Детектор боковика (>= 4 касания).
---
## [008] 2026-10-02 - Синхронизация: коммит + push
**Статус:** PASS
**Автор:** assistant
**Затронуты файлы:** ARCHITECTURE.md, JOURNAL.md
### Что сделано
Коммит 8d507cc, push в main.
---
## [009] 2026-10-02 - Глоссарий: 01-fasy-rynka
**Статус:** PASS
**Автор:** assistant
**Затронуты файлы:** docs/glossary/README.md, docs/glossary/01-fasy-rynka.md, ARCHITECTURE.md
### Что сделано
3 термина: Накопление, Тренд, Распределение.
---
## [010] 2026-10-02 - Утверждение 01-fasy-rynka
**Статус:** PASS
**Автор:** igorsegal + assistant
### Что сделано
3 термина утверждены. Правило: одна запись = одно событие.
---
## [011] 2026-10-03 - Кириллица и ASD-STE100
**Статус:** PASS
**Автор:** igorsegal + assistant
### Что сделано
ARCHITECTURE.md, JOURNAL.md, README.md глоссария, 01-fasy-rynka.md — на кириллице. Коммит 47af576.
---
## [012] 2026-10-03 - Группа 02-struktura
**Статус:** PASS
### Что сделано
5 терминов: Закрепление, IT, ZO, ORT, RM. Формулы: IT+ZO=ORT, ORT+ZO=IT.
---
## [013] 2026-10-03 - Группа 03-signaly
**Статус:** PASS
### Что сделано
3 термина: Дивергенция, Ложный пробой, Ловушка.
---
## [014] 2026-10-03 - Группа 04-urovni
**Статус:** PASS
### Что сделано
4 термина: LU, PU, Поддержка, Сопротивление.
---
## [016] 2026-10-03 - VLevelDetector выполнен
**Статус:** PASS
**Слой:** cluster
**Затронуты файлы:** include/cluster/VLevelDetector.h, src/cluster/VLevelDetector.cpp, tests/test_vlevel_detector.cpp, CMakeLists.txt
### Что сделано
Детектор V-уровня: импульс 2-9 свечей, уровни Фибо 0/50/100%. 5/5 ctest.
---
## [017] 2026-10-04 - VLevelDetector: оптимизация
**Статус:** PASS
**Слой:** cluster
**Предыдущий:** [016]
**Следующий:** [018] - MirrorLevelDetector
### Зачем
Правки от LLM-наблюдателя (DeepSeek через MCP): производительность, валидация, отрицательные цены.
### Что сделано
- find_impl выделен, общий для find и find_last.
- Префиксные суммы. Сложность O(N·max_len) вместо O(N²·max_len²).
- Префиксные массивы строятся только до search_end.
- impulse_direction на префиксных суммах, возвращает 0 при нулевом движении.
- Публичный метод validate().
- pct от |avg|, толеранс от |level| — работает с отрицательными ценами.
- Границы в impulse_direction проверяются явно.
- Выбор длины: самый длинный, при равенстве — сильнее.
### Файлы
- changed: src/cluster/VLevelDetector.cpp
- changed: include/cluster/VLevelDetector.h
- changed: D:\mcp-experiments\mcp_shell.py (OBSERVER_PROMPT ужесточён)
### Результат
5/5 ctest. Повторный analyze: OK, дефектов не найдено.
### Открытые вопросы
1. Порог min_strength подобрать на реальных данных.
2. Прогнать на реальном CSV ClusterDelta.
### Связи
- depends: [016]
- blocks: [018] - MirrorLevelDetector
---
## [019] 2026-10-05 - Вопрос №11 закрыт: направление снятия стопов продавцов — вниз
**Статус:** PASS
**Автор:** igorsegal + assistant
### Что сделано
ask по 28 транскриптам. L02, L08.2, L12, L14, L17, R_L02 подтверждают вниз. В L12 «вверх» — опечатка. forex_kit_rules.md → v6. Осталось 11 вопросов.
---
## [020] 2026-10-05 - Вопрос №7 закрыт: завершение позиции — принцип парности цифр
**Статус:** PASS
**Автор:** igorsegal + assistant
### Что сделано
L16: набор 5-ки, 11-11, 36 → завершение 6-6. Продавец: набор 26, 10-ки → 84-84. Буквального совпадения нет. forex_kit_rules.md → v7. Осталось 10 вопросов.

---
## Как продолжить работу
1. Прочитай JOURNAL.md до конца.
2. Открой ARCHITECTURE.md — проверенное состояние.
3. git log -1.
4. Найди последнюю запись — текущая задача.
5. Продолжай или начни новую, добавив запись снизу.
**Правило:** одна запись = одно событие. Редактирование задним числом запрещено.
**В JOURNAL.md — всегда. В ARCHITECTURE.md — только PASS.**
**Зеркало:** G:\Мой диск\AHexaTrader_BACKUP\spartak\.
**Обновлено:** 2026-10-04 (шаг [017] PASS)
step 018 | Ревизия правил FOREX KIT v5 | Аудит 28 транскриптов, 5 итераций. Закрыты: Bid для стопов покупателей, 80% отработки, приоритет золото→валюта. Остались 17 открытых вопросов, все требуют видеопросмотра. Файлы: docs/forex_kit_rules.md (v5), GAPS.md, AUDIT.md (в .gitignore). | коммит pending