// Таблица из строк: так хранятся исходный текст, ТКО и все таблицы в окне.
#pragma once

#include <string>
#include <vector>

using namespace std;

#include "text.h"

using Row = vector<string>;  // строка таблицы - список ячеек
using Table = vector<Row>;   // таблица - список строк

// Ячейка номер i без пробелов по краям ("" - если такой ячейки нет).
string cell(Row row, int i) {
    if (i >= len(row)) return "";
    return trim(row[i]);
}

bool isEmptyRow(Row row) {
    for (string value : row) {
        if (trim(value) != "") return false;
    }
    return true;
}
