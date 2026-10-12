#pragma once

#include "core/Types.h"
#include <string>
#include <vector>

namespace spartak::data {

class CbinWriter {
public:
    // Записывает бары в CBIN-файл. Возвращает false при ошибке.
    static bool write(const std::string& path,
                      const std::vector<core::Bar>& bars,
                      int period_seconds,
                      int digits,
                      double point);
};

} // namespace spartak::data