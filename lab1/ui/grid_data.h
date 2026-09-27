// Grid: создание таблицы, данные, строки, ширина столбцов.
#pragma once

#include "grid.h"

void Grid::create(HWND parent, int gridId, HFONT gridFont) {
    id = gridId;
    font = gridFont;
    hwnd = CreateWindowExW(0, WC_LISTVIEWW, L"",
                           WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_OWNERDATA | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                           0, 0, 10, 10, parent, (HMENU)(INT_PTR)gridId, GetModuleHandleW(NULL), NULL);
    int style = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP;
    if (showTooltips) style = style | LVS_EX_INFOTIP;
    ListView_SetExtendedListViewStyle(hwnd, style);
    SendMessageW(hwnd, WM_SETFONT, (WPARAM)font, TRUE);
    applyTheme();

    // У ListView нет настройки высоты строки, её задаёт список картинок:
    // пустые картинки высотой 24 пикселя делают строки высотой 24 пикселя.
    ListView_SetImageList(hwnd, comctl.ImageList_Create(1, S(24), ILC_COLOR32, 1, 1), LVSIL_SMALL);

    for (int i = 0; i < viewColumnCount(); i++) {
        LVCOLUMNW column = {};
        column.mask = LVCF_WIDTH;  // подписи столбцов рисуем сами (paintHeader)
        column.cx = S(80);
        ListView_InsertColumn(hwnd, i, &column);
    }

    allGrids.push_back(this);
    // Свой обработчик сообщений поверх стандартного (grid_input.h).
    oldProc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)gridProc);
    if (editable) currentRow = 0;
    refresh();
}

void Grid::applyTheme() {
    ListView_SetBkColor(hwnd, colors.card);
    ListView_SetTextBkColor(hwnd, colors.card);
    ListView_SetTextColor(hwnd, colors.ink);
    applyControlTheme(hwnd, L"DarkMode_Explorer");
    InvalidateRect(hwnd, NULL, TRUE);
}

string Grid::cellText(int row, int col) {
    if (row < 0 || row >= rowCount() || col < 0 || col >= len(rows[row])) return "";
    return rows[row][col];
}

void Grid::setRows(Table newRows) {
    cancelEdit();
    rows = newRows;
    for (int i = 0; i < rowCount(); i++) rows[i].resize(columnCount());  // все строки одной длины
    addEmptyRowIfNeeded();
    if (editable)
        currentRow = min(max(currentRow, 0), rowCount() - 1);
    else
        currentRow = -1;
    refresh();
    if (!editable) ListView_SetItemState(hwnd, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);  // снять выделение
}

// У таблиц для ввода в конце всегда есть пустая строка - чтобы было куда писать.
void Grid::addEmptyRowIfNeeded() {
    bool needRow = rowCount() == 0 || (keepEmptyLastRow && !isEmptyRow(rows.back()));
    if (editable && needRow) rows.push_back(Row(columnCount(), ""));
}

// Показать новые данные.
void Grid::refresh() {
    // Сначала подогнать ширину столбцов под новое число строк, потом сообщить
    // таблице это число. Иначе таблица успевает показать лишнюю горизонтальную
    // полосу прокрутки и уже не убирает её.
    fitColumns();
    ListView_SetItemCountEx(hwnd, rowCount(), editable ? LVSICF_NOSCROLL : 0);
    fitColumns();
    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
}

// Столбцы делят ширину таблицы по долям из widths.
void Grid::fitColumns() {
    if (!hwnd) return;
    RECT box, header;
    GetWindowRect(hwnd, &box);
    GetWindowRect(ListView_GetHeader(hwnd), &header);
    int width = box.right - box.left;
    int height = box.bottom - box.top;

    // Высоту строки можно измерить, только когда строки есть, - запоминаем её.
    RECT firstRow;
    if (ListView_GetItemCount(hwnd) > 0 && ListView_GetItemRect(hwnd, 0, &firstRow, LVIR_BOUNDS))
        rowHeight = firstRow.bottom - firstRow.top;
    int oneRow = rowHeight > 0 ? rowHeight : S(24);

    // Если строки не влезут, справа появится полоса прокрутки - оставляем под неё место.
    bool needScrollBar = rowCount() * oneRow > height - (header.bottom - header.top);
    if (needScrollBar) width -= GetSystemMetrics(SM_CXVSCROLL);
    width -= 1;  // сумма столбцов чуть меньше ширины - тогда нет горизонтальной прокрутки

    if (showRowNumbers) {
        ListView_SetColumnWidth(hwnd, 0, S(40));
        width -= S(40);
    }
    int total = 0;
    for (int w : widths) total += w;
    int used = 0;
    for (int i = 0; i < columnCount(); i++) {
        int columnWidth = width * widths[i] / total;
        if (i == columnCount() - 1) columnWidth = width - used;  // последний добирает остаток
        ListView_SetColumnWidth(hwnd, i + numberColumns(), max(columnWidth, S(20)));
        used += columnWidth;
    }
}

// Выделить ячейку и прокрутить к ней.
void Grid::select(int row, int col) {
    if (row < 0 || row >= rowCount()) return;
    currentRow = row;
    currentCol = min(max(col, 0), columnCount() - 1);
    ListView_SetItemState(hwnd, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
    ListView_SetItemState(hwnd, row, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    ListView_EnsureVisible(hwnd, row, FALSE);
    InvalidateRect(hwnd, NULL, FALSE);
}

void Grid::scrollToTop() {
    if (rowCount() > 0) ListView_EnsureVisible(hwnd, 0, FALSE);
}

// Вставить пустую строку на место at (строки ниже сдвигаются вниз).
void Grid::insertRow(int at) {
    commitEdit();
    at = min(max(at, 0), rowCount());
    rows.insert(rows.begin() + at, Row(columnCount(), ""));
    changed();
    select(at, 0);
    SetFocus(hwnd);
}

void Grid::deleteRow(int at) {
    commitEdit();
    if (at < 0 || at >= rowCount()) return;
    rows.erase(rows.begin() + at);
    changed();
    select(min(at, rowCount() - 1), currentCol);
    SetFocus(hwnd);
}

void Grid::clearCell() {
    if (cellText(currentRow, currentCol) == "") return;
    rows[currentRow][currentCol] = "";
    changed();
}

// Пользователь изменил данные: обновить таблицу и сообщить окну.
void Grid::changed() {
    addEmptyRowIfNeeded();
    refresh();
    onGridChanged(id);
}

// Прямоугольник ячейки (col - номер столбца данных).
RECT Grid::cellBox(int row, int col) {
    int viewColumn = col + numberColumns();
    RECT box;
    ListView_GetSubItemRect(hwnd, row, viewColumn, LVIR_BOUNDS, &box);
    if (viewColumn == 0) box.right = box.left + ListView_GetColumnWidth(hwnd, 0);  // у первого столбца Windows даёт всю строку
    return box;
}

// Сообщения, которые таблица присылает главному окну (WM_NOTIFY).
// true - сообщение обработано, ответ записан в result.
bool Grid::handleNotify(NMHDR* message, LRESULT& result) {
    // Таблица спрашивает текст ячейки, чтобы нарисовать её.
    if (message->code == LVN_GETDISPINFOW) {
        NMLVDISPINFOW* request = (NMLVDISPINFOW*)message;
        int row = request->item.iItem;
        int column = request->item.iSubItem;
        if (request->item.mask & LVIF_TEXT) {
            string text;
            if (column < numberColumns())
                text = to_string(row + 1);  // столбец "№"
            else
                text = cellText(row, column - numberColumns());
            lstrcpynW(request->item.pszText, toWide(text).c_str(), request->item.cchTextMax);
        }
        if (request->item.mask & LVIF_IMAGE) request->item.iImage = -1;  // без картинок
        result = 0;
        return true;
    }
    // Всплывающая подсказка: полный текст строки.
    if (message->code == LVN_GETINFOTIPW) {
        NMLVGETINFOTIPW* request = (NMLVGETINFOTIPW*)message;
        string text = "";
        for (int col = 0; col < columnCount(); col++) {
            string value = cellText(request->iItem, col);
            if (value == "") continue;
            if (text != "") text += "  ·  ";
            text += value;
        }
        lstrcpynW(request->pszText, toWide(text).c_str(), request->cchTextMax);
        result = 0;
        return true;
    }
    // Таблица рисует строки - задаём цвета (grid_paint.h).
    if (message->code == (UINT)NM_CUSTOMDRAW) {
        result = paintCells((NMLVCUSTOMDRAW*)message);
        return true;
    }
    return false;
}
