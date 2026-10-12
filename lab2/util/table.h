#pragma once

#include <string>
#include <vector>

using namespace std;

#include "text.h"

using Row = vector<string>;
using Table = vector<Row>;

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
