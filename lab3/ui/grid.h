// Grid - таблица в окне (на основе стандартной таблицы Windows "ListView").
//
// Данные хранятся в rows. ListView сам данные не хранит: когда нужно
// нарисовать ячейку, он запрашивает её текст (handleNotify в grid_data.h).
//
// Реализация разнесена по файлам:
//   grid_data.h  - создание, данные, строки, ширина столбцов
//   grid_paint.h - цвета ячеек, линии сетки, заголовок столбцов
//   grid_edit.h  - редактирование ячейки и меню по правому клику
//   grid_input.h - мышь и клавиатура
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

// Цвета одной ячейки. Их задаёт окно лабы: например, красный фон у строки с ошибкой.
struct CellColors {
    bool hasBackground = false;
    COLORREF background = 0;
    bool hasText = false;
    COLORREF text = 0;
};

// События таблиц. Сами функции написаны в app/events.h - там окно лабы решает,
// что делать. gridId - номер таблицы (ID_SOURCE, ID_OPS, ...).
void onGridChanged(int gridId);                       // пользователь изменил данные
void onGridRowClicked(int gridId, int row);           // пользователь выбрал строку
CellColors cellColors(int gridId, int row, int col);  // какого цвета ячейка

// Обработчики сообщений Windows для таблицы и поля ввода в ячейке (grid_input.h).
LRESULT CALLBACK gridProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cellEditorProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

class Grid {
public:
    // ---- настройки: задаются до create() ----
    vector<string> headers;         // заголовки столбцов
    vector<int> widths;             // доли ширины столбцов (в сумме - любое число)
    bool editable = false;          // можно ли менять ячейки
    bool keepEmptyLastRow = false;  // в конце всегда пустая строка - для ввода новой
    bool showRowNumbers = false;    // слева столбец "№"
    bool showTooltips = false;      // всплывающая подсказка с полным текстом строки
    string placeholder;             // надпись, пока таблица пуста
    string name;                    // название таблицы для окна "как текст"
    bool labelColumn = false;       // первый столбец - метка (важно для текста, см. grid_text.h)

    // ---- данные ----
    Table rows;
    int currentRow = -1;  // выбранная ячейка
    int currentCol = 0;

    // ---- служебное: то, что нужно Windows ----
    int id = 0;
    HWND hwnd = NULL;              // сама таблица Windows
    HFONT font = NULL;
    int rowHeight = 0;
    WNDPROC oldProc = NULL;        // стандартные обработчики Windows,
    WNDPROC oldEditorProc = NULL;  // которые мы дополняем своими
    HWND editor = NULL;            // поле ввода поверх ячейки (пока редактируем)
    int editRow = -1;
    int editCol = -1;
    bool finishingEdit = false;

    // grid_data.h
    void create(HWND parent, int gridId, HFONT gridFont);
    void applyTheme();
    int rowCount() { return len(rows); }
    int columnCount() { return len(headers); }
    string cellText(int row, int col);
    void setRows(Table newRows);
    void clear() { setRows({}); }
    void refresh();
    void fitColumns();
    void select(int row, int col);
    void scrollToTop();
    void insertRow(int at);
    void deleteRow(int at);
    void clearCell();
    bool handleNotify(NMHDR* message, LRESULT& result);
    void addEmptyRowIfNeeded();
    void changed();
    RECT cellBox(int row, int col);

    // Столбцы ListView = столбец "№" (если он есть) + столбцы данных.
    int numberColumns() { return showRowNumbers ? 1 : 0; }
    int viewColumnCount() { return columnCount() + numberColumns(); }
    int toDataColumn(int viewColumn) { return max(viewColumn - numberColumns(), 0); }

    // grid_paint.h
    LRESULT paintCells(NMLVCUSTOMDRAW* draw);
    void drawCellLines(HDC canvas, int row);
    LRESULT paintHeader(NMCUSTOMDRAW* draw);
    void paintPlaceholder();

    // grid_edit.h
    void beginEdit(wstring typed);
    void finishEdit(bool save, int moveRows, int moveCols, bool focusGrid);
    void commitEdit() { finishEdit(true, 0, 0, false); }
    void cancelEdit() { finishEdit(false, 0, 0, true); }
    void moveColumn(int delta);
    void showContextMenu(LPARAM lParam);

    // grid_input.h
    bool onKeyDown(WPARAM key);

    // grid_text.h
    string toText();
    Table fromText(string text);
    void editAsText();
};

// Все таблицы окна - чтобы по окну Windows (HWND) найти нашу таблицу.
vector<Grid*> allGrids;

Grid* findGrid(HWND hwnd) {
    for (Grid* grid : allGrids) {
        if (grid->hwnd == hwnd) return grid;
    }
    return NULL;
}

// Координаты мыши из сообщения Windows.
int mouseX(LPARAM lParam) { return (short)LOWORD(lParam); }
int mouseY(LPARAM lParam) { return (short)HIWORD(lParam); }

#include "grid_data.h"
#include "grid_edit.h"
#include "grid_input.h"
#include "grid_paint.h"
#include "grid_text.h"
