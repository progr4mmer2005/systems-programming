#pragma once

#include <algorithm>
#include <string>
#include <vector>

using namespace std;

#include "../util/table.h"
#include "draw.h"
#include "fonts.h"
#include "metrics.h"
#include "theme.h"

struct CellColors {
    bool hasBackground = false;
    COLORREF background = 0;
    bool hasText = false;
    COLORREF text = 0;
};

void onGridChanged(int gridId);
void onGridRowClicked(int gridId, int row);
CellColors cellColors(int gridId, int row, int col);

const int NUMBER_COLUMN_WIDTH = 40;

LRESULT CALLBACK gridProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cellEditorProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

class Grid {
public:
    vector<string> headers;
    vector<int> widths;
    bool editable = false;
    bool keepEmptyLastRow = false;
    bool showRowNumbers = false;
    bool showTooltips = false;
    string placeholder;
    string name;
    bool labelColumn = false;
    bool fitText = false;

    Table rows;
    int currentRow = -1;
    int currentCol = 0;

    int id = 0;
    HWND hwnd = NULL;
    HFONT font = NULL;
    WNDPROC oldProc = NULL;
    WNDPROC oldEditorProc = NULL;
    HWND editor = NULL;
    int editRow = -1;
    int editCol = -1;
    bool finishingEdit = false;

    void create(HWND parent, int gridId, HFONT gridFont);
    void applyTheme();
    int rowCount() { return len(rows); }
    int columnCount() { return len(headers); }
    string cellText(int row, int col);
    void setRows(Table newRows);
    void clear() { setRows({}); }
    void refresh();
    void fitColumns();
    int longestText(int col);
    void select(int row, int col);
    void unselect();
    void scrollToTop();
    void showRow(int row);
    void insertRow(int at);
    void deleteRow(int at);
    void clearCell();
    bool handleNotify(NMHDR* message, LRESULT& result);
    void addEmptyRowIfNeeded();
    void changed();
    RECT cellBox(int row, int col);

    int numberColumns() { return showRowNumbers ? 1 : 0; }
    int viewColumnCount() { return columnCount() + numberColumns(); }
    int toDataColumn(int viewColumn) { return max(viewColumn - numberColumns(), 0); }

    LRESULT paintCells(NMLVCUSTOMDRAW* draw);
    void drawCellLines(HDC canvas, int row);
    void paintPlaceholder();

    void beginEdit(wstring typed);
    void finishEdit(bool save, int moveRows, int moveCols, bool focusGrid);
    void commitEdit() { finishEdit(true, 0, 0, false); }
    void cancelEdit() { finishEdit(false, 0, 0, true); }
    void moveColumn(int delta);
    void showContextMenu(LPARAM lParam);

    bool onKeyDown(WPARAM key);

    string toText();
    Table fromText(string text);
    void editAsText();
};

vector<Grid*> allGrids;

Grid* findGrid(HWND hwnd) {
    for (Grid* grid : allGrids) {
        if (grid->hwnd == hwnd) return grid;
    }
    return NULL;
}

int mouseX(LPARAM lParam) { return (short)LOWORD(lParam); }
int mouseY(LPARAM lParam) { return (short)HIWORD(lParam); }

#include "grid_data.h"
#include "grid_edit.h"
#include "grid_input.h"
#include "grid_paint.h"
#include "grid_text.h"
