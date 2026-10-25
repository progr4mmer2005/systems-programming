#pragma once

#include "actions.h"
#include "state.h"

void onGridChanged(int gridId) {
    if (gridId == ID_SOURCE || gridId == ID_OPS) onInputChanged();
}

int sourceRowOf(int gridId, int row) {
    if (row < 0) return -1;
    if (gridId == ID_SOURCE) return row;
    if (gridId == ID_SUPPORT && row < len(pass1.lines)) return pass1.lines[row].row;
    if (gridId == ID_SYMBOLS && row < len(pass1.symbols)) return pass1.symbols[row].row;
    if (gridId == ID_RELOC && row < len(pass2.relocations)) return pass2.relocations[row].row;
    if (gridId == ID_EXTNAMES && row < len(pass2.externalNames)) return pass2.externalNames[row].row;
    if (gridId == ID_EXTREFS && row < len(pass2.externalRefs)) return pass2.externalRefs[row].row;
    if (gridId == ID_HEADER && row == 0 && len(pass1.lines) > 0) return pass1.lines[0].row;
    if (gridId == ID_CODE && row < len(moduleRecords)) return moduleRecords[row].sourceRow;
    return -1;
}

void selectBySource(Grid& grid, int sourceRow) {
    for (int row = 0; row < grid.rowCount(); row++) {
        if (sourceRowOf(grid.id, row) == sourceRow) {
            grid.select(row, grid.currentCol);
            return;
        }
    }
    grid.unselect();
}

void showSourceRow(int sourceRow, int fromGridId) {
    if (sourceRow < 0) return;
    if (fromGridId != ID_SOURCE) sourceGrid.select(sourceRow, fromGridId == ID_SYMBOLS || fromGridId == ID_HEADER ? 0 : 1);
    for (Grid* grid : {&supportGrid, &symbolsGrid, &headerGrid, &relocGrid, &extNamesGrid, &extRefsGrid, &codeGrid}) {
        if (grid->id != fromGridId) selectBySource(*grid, sourceRow);
    }
}

void onGridRowClicked(int gridId, int row) {
    if (gridId == ID_ERRORS1 || gridId == ID_ERRORS2) {
        vector<Error> errors = gridId == ID_ERRORS1 ? pass1.errors : pass2.errors;
        showErrorRow(errors, row);
        if (row < len(errors) && errors[row].place == "source") showSourceRow(errors[row].row, ID_SOURCE);
        return;
    }
    if (gridId == ID_OPS) return;
    showSourceRow(sourceRowOf(gridId, row), gridId);
}

CellColors cellColors(int gridId, int row, int col) {
    CellColors result;

    bool badSource = gridId == ID_SOURCE && row < len(badSourceRows) && badSourceRows[row];
    bool badOps = gridId == ID_OPS && row < len(badOpsRows) && badOpsRows[row];
    if (badSource || badOps) {
        result.hasBackground = true;
        result.background = colors.errorBackground;
    }

    if (gridId == ID_ERRORS1 || gridId == ID_ERRORS2) {
        result.hasText = true;
        result.text = col == 0 ? colors.muted : colors.error;
    }

    if (gridId == ID_CODE && row < len(moduleRecords)) {
        if (moduleRecords[row].unresolved) {
            result.hasBackground = true;
            result.background = colors.errorBackground;
        } else if (col == 0 || moduleRecords[row].reserve) {
            result.hasText = true;
            result.text = colors.muted;
        }
    }
    return result;
}
