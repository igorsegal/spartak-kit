// =============================================================================
//  spartak-kit :: data/ClusterCsvReader.h
//  Парсер CSV-выгрузок ClusterDelta для FOREX KIT.
//
//  Формат (реальный пример):
//      OPEN_DATE;OPEN_TIME;OPEN;HIGH;LOW;CLOSE;VOLUME;DELTA;ASK;BID
//      02.01.2026;01:00;1.2719;1.2719;1.2713;1.2717;12;4;8;4
//
//  Разделитель       : ';'
//  Формат даты       : DD.MM.YYYY
//  Формат времени    : HH:MM
//  Разделитель дробной части : '.'
//  Спред в файле отсутствует -> spread = 0
// =============================================================================
#pragma once
#include "core/Types.h"
#include <cstdint>
#include <fstream>
#include <string>
namespace spartak::data {
struct CsvReaderOptions {
    char delimiter = ';';
    bool has_header = true;
    // Смещение таймзоны в часах (0 = UTC). Если файл в UTC — оставить 0.
    int timezone_offset_hours = 0;
};
class ClusterCsvReader {
public:
    explicit ClusterCsvReader(CsvReaderOptions opts = {});
    // Открывает файл. Если has_header == true — пропускает первую строку.
    bool open(const std::string& path);
    // Читает следующий бар. Возвращает false при EOF или ошибке.
    bool read_next(core::Bar& out);
    void close();
    bool    is_open()   const noexcept { return stream_.is_open(); }
    int64_t bars_read() const noexcept { return bars_read_; }
private:
    CsvReaderOptions opts_;
    std::ifstream    stream_;
    int64_t          bars_read_ = 0;
};
} // namespace spartak::data