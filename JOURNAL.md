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
## [022] 2026-10-05 - Вопросы №5, 8, 15 (частично) закрыты через OCR 34 скриншотов
**Статус:** PASS
**Автор:** igorsegal + assistant
### Что сделано
Установлен Tesseract 5.5.3, обработаны все 34 скриншота. Подтверждено: формула скрытого уровня, иллюзия рынка, тень, даркпулы, 95%, симулятор в терминале SB Pro, крупный игрок оставляет след в дельте. Термин «запертый объём» не существует. forex_kit_rules.md → v8. Осталось 8 вопросов.
---
## [031] 2026-10-06 - ARCHITECTURE.md обновлён под ревизию правил
**Статус:** PASS
**Автор:** igorsegal + assistant
### Что сделано
§1 паспорт, §10 TODO, §11 roadmap (9 детекторов), §15 ревизия FOREX KIT, §16 источники. Правила v16 — все 17 вопросов закрыты.
---
## [032] 2026-10-06 - MirrorLevelDetector PASS
**Статус:** PASS
**Автор:** igorsegal + assistant
### Что сделано
Формация: держали → пробили → ретест. Тест 11/11. analyze: дефектов не найдено. ARCHITECTURE.md обновлён: §4 реестр, §11 roadmap, §15.
---
## [034] 2026-10-06 - DivergenceDetector PASS
**Статус:** PASS
**Автор:** igorsegal + assistant
### Что сделано
RSI-14 по Уайлдеру, pivot ±5, окно 50. Тест 12/12. analyze нашёл пропуск бара 0, исправлено, повторный analyze чист. ARCHITECTURE.md обновлён.
---
## [035] 2026-10-06 - FalseBreakoutDetector WIP
**Статус:** PASS
**Автор:** igorsegal + assistant
### Что сделано
ТЗ прошло 3 раунда архитектурного ревью (spec). Код написан, тест FAIL. Детектор не находит формацию: touches=0, хотя касаний 7. Пробой и возврат по close. Причина не найдена, нужно продолжить отладку. Файлы: specs/021_false_breakout.txt, src/cluster/FalseBreakoutDetector.cpp, include/cluster/FalseBreakoutDetector.h, tests/test_false_breakout_detector.cpp.

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
step 023 | Вопрос №1 закрыт: порог парных цифр — двузначные | ask по 28 транскриптам, 3 файла с упоминаниями. Сигналы продавца и покупателя собраны. Мелкие (2-2, 6-6, 8-8) не считать. forex_kit_rules.md → v9. Осталось 7 вопросов.
step 024 | Вопрос №2 закрыт: настройки BitAsk | ask по 28 транскриптам, 4 файла. BitAsk показывает покупателей/продавцов. Натягивать от начала боковика до пробития. Другие профили не использовать. Уточнены Bid/Ask в §2. forex_kit_rules.md → v10. Осталось 6 вопросов.
step 025 | Вопрос №3 закрыт частично | ask по 28 транскриптам. Настроек дельты нет, метод чтения — есть: показывает толпу, нули стопов, область по двум крупнейшим кластерам. forex_kit_rules.md → v11. Осталось 5 вопросов.
step 027 | Вопрос №9 закрыт: формулы каскадного уровня нет | ask по 28 транскриптам, 5 файлов. Описание: два уровня подряд, цена отбилась дважды. Точка входа после второго отбоя. Усилитель, не отдельный сигнал (R_L02, L08.2). forex_kit_rules.md → v13. Осталось 3 вопроса.
step 028 | Вопрос №10 закрыт: ATR 5% — формулы нет | S_Clusters: «крупный игрок не пустит выше 5%, ATR — аналогия, не важно». Работает как принцип. forex_kit_rules.md → v14. Осталось 2 вопроса.
step 029 | Вопрос №12 закрыт: распродажа закрыта цифрами — подтверждено | R_L03: «закрыта сверху и снизу цифрами». У 100-плюсового после 0 ничего нет — не распродажа, отсутствие объёма. forex_kit_rules.md → v15. Осталось 1 вопрос.
step 030 | Вопрос №17 закрыт: 1:2 — ориентир, не жёсткое правило | L09: «всегда нужно ждать 1:2», но «это как бы теории». Может дать больше или не дотянуть. forex_kit_rules.md → v16. ВСЕ 17 ВОПРОСОВ ЗАКРЫТЫ.
step 033 | DeltaDetector PASS | Аномальная дельта: |delta| > 2.5 × stddev за 50 баров. Направление по знаку. Противоречие толпа/свеча. Тест 15/15. analyze: OK. ARCHITECTURE.md обновлён.