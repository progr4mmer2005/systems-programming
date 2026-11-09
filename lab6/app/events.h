#pragma once

#include "actions.h"
#include "state.h"

void onGridChanged(int gridId) {
    if (gridId == ID_SOURCE || gridId == ID_OPS) onInputChanged();
}

int sourceRowOf(int gridId, int row) {
    if (row < 0) return -1;
    if (gridId == ID_SOURCE) return row;
    if (gridId == ID_SYMBOLS) { vector<Symbol> symbols = visibleSymbols(assembly); if (row < len(symbols)) return symbols[row].row; }
    if (gridId == ID_HEADER) { if (row < len(assembly.sections)) return assembly.sections[row].startRow; if (row == len(assembly.sections)) return assembly.startRow; }
    if (gridId == ID_RELOC && row < len(assembly.relocations)) return assembly.relocations[row].row;
    if (gridId == ID_EXTNAMES) {
        vector<Symbol> names = externalNames(assembly);
        if (row < len(names)) return names[row].row;
    }
    if (gridId == ID_EXTREFS && row < len(assembly.extRefs)) return assembly.extRefs[row].row;
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
    for (Grid* grid : {&symbolsGrid, &headerGrid, &relocGrid, &extNamesGrid, &extRefsGrid, &codeGrid}) {
        if (grid->id != fromGridId) selectBySource(*grid, sourceRow);
    }
}

void onGridRowClicked(int gridId, int row) {
    if (gridId == ID_ERRORS) {
        showErrorRow(assembly.errors, row);
        if (row < len(assembly.errors) && assembly.errors[row].place == "source") showSourceRow(assembly.errors[row].row, ID_SOURCE);
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
    if (gridId == ID_SOURCE && row == nextSourceRow) {
        result.hasBackground = true;
        result.background = colors.current;
    }

    if (gridId == ID_ERRORS) {
        result.hasText = true;
        result.text = col == 0 ? colors.muted : colors.error;
    }

    vector<Symbol> symbols = visibleSymbols(assembly);
    if (gridId == ID_SYMBOLS && row < len(symbols) && !symbols[row].defined) {
        result.hasText = true;
        result.text = colors.muted;
    }

    if (gridId == ID_CODE && row < len(moduleRecords)) {
        ModuleRecord record = moduleRecords[row];
        if (record.unresolved) {
            result.hasBackground = true;
            result.background = colors.errorBackground;
        } else if (col == 0 || record.reserve || record.waiting) {
            result.hasText = true;
            result.text = colors.muted;
        }
    }

    if (gridId == ID_EXTNAMES) {
        vector<Symbol> names = externalNames(assembly);
        if (row < len(names) && !names[row].defined) {
            result.hasText = true;
            result.text = colors.muted;
        }
    }
    return result;
}
