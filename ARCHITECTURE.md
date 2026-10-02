# SPARTAK KIT :: ARCHITECTURE
Master-dokument proekta. Obnovlyaetsya pri kazhdom izmenenii struktury.
Sozdan: 2026-10-02.
---
## 1. Pasport
- **Nazvanie:** SPARTAK KIT
- **Naznachenie:** C++20 bektester torgovoy sistemy FOREX KIT.
- **Istochnik sistemy:** kurs FOREX KIT (28 video), docs/forex_kit_rules.md.
- **Istochnik dannyh:** ClusterDelta CSV (https://my.clusterdelta.com/files).
- **Stek:** C++20, CMake >= 3.20, MSVC 2022+.
- **Namespace:** spartak::core, spartak::data, spartak::cluster, ...
- **Repozitoriy:** github.com/igorsegal/spartak-kit
---
## 2. Pravila repozitoriya
### 2.1. Reestr faylov
Lyuboy novyy fayl v `include/`, `src/`, `tests/` DOLZhEN byt opisan v razdele 4.
Novyy fayl bez zapisi v reestre -> CI krasnyy.
### 2.2. Kommity
- Odin kommit = odno logicheskoe izmenenie.
- Soobshchenie kommita: `[sloy]: kratkoe opisanie`.
- Primery: `data: add ClusterDelta CSV reader`, `cluster: add FootprintDetector`.
### 2.3. Vetki
- `main` -- stabilnaya.
- Push v `main` tolko cherez PR.

### 2.4. Reestr bez zagluzhek
Esli v tablitse reestra est' stroki - fraza "Poka pusto" udalyaetsya.
Zagluzhka i zapisi ne sosushchestvuyut.
### 2.5. Distsiplina statusov
Dopustimo tolko pyat' znacheniy:
- `plan`     - zaplanirovano, rabota ne nachata
- `WIP`      - idet pryamo seychas
- `PASS`     - testy proshli, zafiksirovano
- `FAIL`     - testy upali, otkat obyazatelen
- `ROLLBACK` - otkatili, shag zakryt
Drugih znacheniy net.
### 2.6. Bystryy start vsegda aktualen
Komandy sborki i testov zhivut v odnom meste - sec."Bystryy start".
Menyaesh' komandy - obnovlyaesh' blok v tom zhe commite.
---
## 3. Planiruemaya struktura
    include/core/        tipy, konstanty, konfig
    include/data/        chtenie XFBAR i ClusterDelta CSV
    include/cluster/     detektory signalov FOREX KIT
    src/                 realizatsii
    tests/               smoke-testy
    docs/                pravila sistemy, ARCHITECTURE, ROADMAP
    tools/               konvertery, CLI-utility
---
## 4. Reestr faylov
Reestr zapolnyaetsya po mere zakrytiya shagov (PASS).
| Put | Rol | Status |
|---|---|---|
| `include/core/Types.h`         | Bazovye tipy (Bar, KitSignal)     | gotov |
| `include/data/BarStream.h`     | Potok barov (Synthetic/Vector)    | gotov |
| `src/data/BarStream.cpp`       | Realizatsiya BarStream              | gotov |
| `include/data/DataSanitizer.h` | Sanitayzer (potokovyy, BarStream) | gotov |
| `src/data/DataSanitizer.cpp`   | Realizatsiya DataSanitizer          | gotov |
Statusy: `gotov`, `v rabote`, `zaglushka`, `musor`.
---
## 5. Potok dannyh
Poka ne opredelen. Budet zapolnen posle sozdaniya sloya data/.
Planiruemaya shema:
    ClusterDelta CSV
      -> data::ClusterCsvReader
      -> data::BarStream (rasshirennyy: delta, ask, bid)
      -> cluster::* (detektory FOREX KIT)
      -> engine::BacktestPlayer
      -> BacktestReport
---
## 6. Strategiya FOREX KIT
Pravila sistemy v otdelnom dokumente: `docs/forex_kit_rules.md`.
Kratkaya svodka signalov:
- Stop-losy pokupateley / prodavtsov.
- Rasprodazhi.
- Bokoviki (>= 4 kasaniya).
- V-urovni, zerkalnye urovni, plity.
- Krupnyy igrok (parnye tsifry).
- Delta, total-delta.
---
## 7. Optimizatsiya
Poka ne opredelena. Parametry dlya optimizatsii:
- Razmer parnyh tsifr krupnogo igroka.
- Razmer oblasti stopov.
- Porog delty.
---
## 8. Zhurnal rezultatov
Shema poka ne opredelena. Minimum poley:
`instrument, period, signal_type, direction, entry, exit, pnl, comment`.
---
## 9. CI
Planiruetsya GitHub Actions:
- Sborka na kazhdyy push.
- Smoke-testy.
- Proverka reestra (novyy fayl bez zapisi -> fail).
---
## 10. TODO
- [ ] Sozdat docs/forex_kit_rules.md -- polnye pravila sistemy.
- [ ] Sozdat include/core/Types.h -- bazovye tipy.
- [ ] Sozdat CMakeLists.txt.
- [ ] Realizovat data::ClusterCsvReader.
- [ ] Realizovat detektory signalov.
- [ ] Nastroit CI.---
## Protokol raboty i zerkalirovanie
**Zhurnal:** `JOURNAL.md` - polnaya hronologiya shagov. Novye zapisi snizu.
**Istochnik pravdy:** repozitoriy na GitHub.
**Zerkalo:** `G:\Moy disk\AHexaTrader_BACKUP\spartak\` - kopii `JOURNAL.md` i `ARCHITECTURE.md`.
### Pravilo obnovleniya etogo fayla
`ARCHITECTURE.md` obnovlyaetsya **tolko posle prohozhdeniya testa (PASS)**.
Promezhutochnye i neudachnye shagi fiksiruyutsya isklyuchitelno v `JOURNAL.md`.
### Pravilo raboty s istochnikami
Rabotaem tolko s `spartak-kit` i `spartak`.
Iz "Spartaka" perenosim proverennye moduli obrabotki dannyh, upravleniya
pozitsiey i dvizhka. Logika TAP ne perenositsya.
### Sinhronizatsiya
robocopy . "G:\Moy disk\AHexaTrader_BACKUP\spartak" JOURNAL.md ARCHITECTURE.md /XO---
## Realizovannye moduli (proshli smoke-test)
### data/DataSanitizer  [shag 002-004, PASS]
- Naznachenie: otsechenie neregulyarnogo prefiksa potoka barov
  (Daily-prefiks, gepy) -- strategiya startuet ot pervogo regulyarnogo bara.
- Fayly: `include/data/DataSanitizer.h`, `src/data/DataSanitizer.cpp`
- Istochnik: igorsegal/spartak (bez izmeneniy logiki).
### data/BarStream  [shag 003-004, PASS]
- Naznachenie: edinaya tochka podachi barov v bektest.
- Rezhimy: Synthetic (port iz Spartaka), Vector (spartak-kit extension).
- File mode ne perenesen -- zamenyaetsya na ClusterCsvReader v [005].
- Fayly: `include/data/BarStream.h`, `src/data/BarStream.cpp`
### tests/smoke_test  [shag 003-004, PASS]
- 5 testov DataSanitizer: pustoy potok, regulyarnyy potok,
  neregulyarnyy prefiks, otsutstvie regulyarnogo hvosta, Synthetic mode.
- Fayl: `tests/smoke_test.cpp`

### data/ClusterCsvReader  [shag 005, PASS]
- Naznachenie: parsing CSV-vygruzok ClusterDelta (format FOREX KIT).
- Format: `OPEN_DATE;OPEN_TIME;OPEN;HIGH;LOW;CLOSE;VOLUME;DELTA;ASK;BID`.
- Osobennosti: data DD.MM.YYYY, vremya HH:MM, timestamp v UTC,
  spred = 0 (v formate otsutstvuet), optsiya timezone_offset_hours.
- Fayly: `include/data/ClusterCsvReader.h`, `src/data/ClusterCsvReader.cpp`
### tests/test_cluster_csv  [shag 005, PASS]
- Smoke-test parsera na 5 barah iz realnoy vygruzki.
- Fayl: `tests/test_cluster_csv.cpp`
- Dannye: `tests/data/sample.csv`

### tests/csv_stats  [shag 005c, PASS]
- Diagnosticheskaya utilita: statistika po lyubomu CSV-faylu ClusterDelta.
- Pechataet: kol-vo barov, first/last ts, span, min low, max high, sum volume/delta/ask/bid.
- Fayl: `tests/csv_stats.cpp`

### data/BarStream.FileMode  [shag 006, PASS]
- Naznachenie: chitat' CSV ClusterDelta napryamuyu cherez BarStream.
- Rezhim: StreamMode::File. Konstruktor BarStream(const std::string& csv_path)
  chitaet ves' fayl cherez ClusterCsvReader v source_, dalee rabotaet
  Vetka Vector (bez duplirovaniya koda v next()).
- Sanity: DataSanitizer.run() na 5-bar sample.csv - ok=false, bars_scanned=5
  (ozhidaemo: confirm_bars=10, barov vsego 5).
- Testy: tests/test_barstream_file.cpp - 3/3 ctest proshli.
- Fayly: include/data/BarStream.h, src/data/BarStream.cpp.---
## 11. Roadmap detektorov
Poryadok - po sec.9 "Torgovyy process" i sec.7 "Mnogofaktornyy analiz"
iz docs/forex_kit_rules.md.
### Dostupno na tekushchih dannyh (bar-level ClusterDelta CSV)
OHLC + DELTA + obshchie ASK/BID na bar.
| # | Detektor           | Formaciya                        | Rezhim    | Shag |
|---|--------------------|----------------------------------|-----------|------|
| 1 | RangeDetector      | Bokovik >= 4 kasaniya (sec.6.1)     | okno      | 007  |
| 2 | VLevelDetector     | V-uroven', impuls 2-9 svechey (sec.6.3) | okno  | 008  |
| 3 | MirrorLevelDetector| Zerkalnyy uroven' (sec.6.5)         | okno      | 009  |
| 4 | DeltaDetector      | Delta / total-delta (sec.7)         | okno      | 010  |
### Trebuet footprint-dannyh (price-level clusters)
Klastery po kazhdomu tsenovomu urovnyu vnutri bara. V tekushchey vygruzke
ClusterDelta otsutstvuyut. Zhdem istochnik.
| # | Detektor            | Formaciya                       |
|---|---------------------|---------------------------------|
| 5 | LiquidationDetector | Rasprodazha (sec.3, sec.6.4)          |
| 6 | StopLossDetector    | Stop-lossy pokupateley/prodavtsov (sec.3, sec.6.2) |
| 7 | SlabDetector        | Plita (sec.6.6)                    |
| 8 | LargePlayerDetector | Krupnyy igrok, parnye tsifry (sec.4) |
| 9 | ShadowDetector      | Ten' (sec.6.7)                     |
### Zavisimost'
- CompositeSignal (sec.7): minimum 2-3 detektora iz gruppy 1-4 dolzhny rabotat'.---
## Bystryy start
    cmake -S . -B build
    cmake --build build --config Debug
    cd build
    ctest -C Debug --output-on-failure
    cd ..
Dobavil novyy fayl v include/ src/ tests/ - vnesi ego v sec.4 Reestr faylov.
Dobavil detektor - vnesi v sec.11 Roadmap.---
## 12. Istochnik dannyh
**Pravilo 12.1. Bazovyy istochnik - .bin XFBAR.**
Lokalnye binarnye fayly v korne:
    D:\AHexaTrader\1DataFiles\raw\<SYMBOL>\<SYMBOL>_<TF>.bin
Naprimer: D:\AHexaTrader\1DataFiles\raw\EURUSD\EURUSD_M5.bin
**Pravilo 12.2. Format .bin.**
Struktura zagolovka i svechi - v docs/format_bin.md.
Izmenenie struktury = novaya versiya v pole magic. Bez novoy versii - ne menyat.
**Pravilo 12.3. Dopolnitelnyy istochnik - ClusterDelta CSV.**
Ispolzuetsya tolko dlya DELTA / ASK / BID. Ne zamenyaet .bin i ne
schitaetsya bazovym.
**Pravilo 12.4. Setevye istochniki zapreshcheny.**
V backteste zapreshcheny HTTP / API / lyubye online-istochniki.
Dannye dolzhny byt na diske do starta progonki.---
## 13. Terminologiya (Glossary)
Vse terminy torgovoy sistemy opisany v docs/glossary/.
Shablon, statusy i pravila vedeniya - v docs/glossary/README.md.
### Reestr faylov glossariya
| Fayl | Soderzhanie | Status |
|---|---|---|
| `README.md` | Shablon, statusy, pravila vedeniya | utverzhdeno |
| `01-fasy-rynka.md` | Nakoplenie, trend, raspredelenie | utverzhdeno |
| `02-struktura.md` | IT, ZO, ORT, RM, zakreplenie | plan |
| `03-signaly.md` | Divergenciya, lozhnyy proboy, lovushka | plan |
| `04-urovni.md` | LU, PU, podderzhka, soprotivlenie | plan |
| `05-obyom.md` | Dobor, uderzhanie, pereliv | plan |
| `06-formatcii.md` | Bokovik, V-uroven, zerkalnyy, plita | plan |
| `07-futprint.md` | Terminy zhduchie footprint-dannyh | plan |
### Statusy terminov
- `chernovik` - zapisan, ne proveren polzovatelem
- `utverzhdeno` - proveren i utverzhden
Termin iz gruppy ne perehodit v `utverzhdeno`, poka polzovatel ne podtverdil.
