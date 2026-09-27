// События таблиц (объявлены в ui/grid.h): что делает окно лабы,
// когда в таблице что-то происходит.
#pragma once

#include "actions.h"
#include "state.h"

// Пользователь изменил исходный текст или ТКО.
void onGridChanged(int gridId) {
    if (gridId == ID_SOURCE || gridId == ID_OPS) onInputChanged();
}

// Пользователь выбрал строку. В таблицах результатов подсвечиваем строку
// исходного текста, из которой эта строка получилась.
void onGridRowClicked(int gridId, int row) {
    if (gridId == ID_SUPPORT && row < len(pass1.lines)) sourceGrid.select(pass1.lines[row].row, 1);
    if (gridId == ID_SYMBOLS && row < len(pass1.symbols)) sourceGrid.select(pass1.symbols[row].row, 0);
    if (gridId == ID_CODE && row < len(pass2.code)) sourceGrid.select(pass2.code[row].row, 1);
    if (gridId == ID_RELOC && row < len(pass2.relocations)) sourceGrid.select(pass2.relocations[row].row, 1);
    if (gridId == ID_EXTNAMES && row < len(pass2.externalNames)) sourceGrid.select(pass2.externalNames[row].row, 0);
    if (gridId == ID_EXTREFS && row < len(pass2.externalRefs)) sourceGrid.select(pass2.externalRefs[row].row, 2);
    if (gridId == ID_ERRORS1) showErrorRow(pass1.errors, row);
    if (gridId == ID_ERRORS2) showErrorRow(pass2.errors, row);
}

// Цвета ячеек.
CellColors cellColors(int gridId, int row, int col) {
    CellColors result;

    // Строки с ошибками - красный фон.
    bool badSource = gridId == ID_SOURCE && row < len(badSourceRows) && badSourceRows[row];
    bool badOps = gridId == ID_OPS && row < len(badOpsRows) && badOpsRows[row];
    if (badSource || badOps) {
        result.hasBackground = true;
        result.background = colors.errorBackground;
    }

    // Списки ошибок: "Где" - серым, текст ошибки - красным.
    if (gridId == ID_ERRORS1 || gridId == ID_ERRORS2) {
        result.hasText = true;
        result.text = col == 0 ? colors.muted : colors.error;
    }

    // Двоичный код: неопределённая метка - красным фоном; резерв и столбец "Команда" - серым.
    if (gridId == ID_CODE && row < len(pass2.code)) {
        CodeLine line = pass2.code[row];
        if (line.unresolved) {
            result.hasBackground = true;
            result.background = colors.errorBackground;
        } else if (line.reserve || col == 2) {
            result.hasText = true;
            result.text = colors.muted;
        }
    }
    return result;
}
