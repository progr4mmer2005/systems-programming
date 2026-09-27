// Grid: как рисуются ячейки, линии сетки и заголовок столбцов.
//
// ListView рисует себя сам, но перед строкой и ячейкой запрашивает цвета.
#pragma once

#include "grid.h"

LRESULT Grid::paintCells(NMLVCUSTOMDRAW* draw) {
    int stage = draw->nmcd.dwDrawStage;
    int row = (int)draw->nmcd.dwItemSpec;

    // Выделенную строку подсвечиваем сами - стандартную подсветку Windows отключаем.
    draw->nmcd.uItemState &= ~(CDIS_SELECTED | CDIS_FOCUS | CDIS_HOT);

    // Начало рисования таблицы.
    if (stage == CDDS_PREPAINT) return CDRF_NOTIFYITEMDRAW;

    // Перед строкой.
    if (stage == CDDS_ITEMPREPAINT) return CDRF_NOTIFYSUBITEMDRAW | CDRF_NOTIFYPOSTPAINT;

    // После строки: дорисовать линии сетки.
    if (stage == CDDS_ITEMPOSTPAINT) {
        drawCellLines(draw->nmcd.hdc, row);
        return CDRF_DODEFAULT;
    }

    // Перед ячейкой: выбрать цвет фона и текста.
    if (stage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM)) {
        if (draw->iSubItem < numberColumns()) {  // столбец "№"
            draw->clrText = row == currentRow ? colors.accent : colors.muted;
            draw->clrTextBk = colors.header;
            return CDRF_NEWFONT;
        }
        int col = draw->iSubItem - numberColumns();
        COLORREF background = row % 2 == 1 ? colors.zebra : colors.card;  // строки через одну чуть темнее
        COLORREF text = colors.ink;

        CellColors custom = cellColors(id, row, col);  // что скажет окно лабы (app/events.h)
        if (custom.hasBackground) background = custom.background;
        if (custom.hasText) text = custom.text;

        if (row == currentRow) {
            bool isCurrentCell = editable && col == currentCol && GetFocus() == hwnd;
            if (isCurrentCell)
                background = colors.cellSelected;
            else if (!custom.hasBackground)
                background = colors.rowSelected;
        }
        draw->clrText = text;
        draw->clrTextBk = background;
        return CDRF_NEWFONT;
    }
    return CDRF_DODEFAULT;
}

// Линии между ячейками и синяя рамка вокруг выбранной ячейки (как в Excel).
void Grid::drawCellLines(HDC canvas, int row) {
    RECT rowBox;
    if (!ListView_GetItemRect(hwnd, row, &rowBox, LVIR_BOUNDS)) return;

    RECT bottomLine = {rowBox.left, rowBox.bottom - 1, rowBox.right, rowBox.bottom};
    fill(canvas, bottomLine, colors.grid);

    int x = rowBox.left;
    for (int column = 0; column < viewColumnCount() - 1; column++) {
        x += ListView_GetColumnWidth(hwnd, column);
        RECT line = {x - 1, rowBox.top, x, rowBox.bottom};
        fill(canvas, line, colors.grid);
    }

    if (editable && row == currentRow && GetFocus() == hwnd) {
        RECT box = cellBox(row, currentCol);
        frame(canvas, box, colors.accent);
        InflateRect(&box, -1, -1);  // вторая рамка на пиксель внутрь - толщина 2
        frame(canvas, box, colors.accent);
    }
}

// Заголовок столбцов рисуем сами - чтобы он был в цветах темы.
LRESULT Grid::paintHeader(NMCUSTOMDRAW* draw) {
    if (draw->dwDrawStage == CDDS_PREPAINT) {
        fill(draw->hdc, draw->rc, colors.header);
        return CDRF_NOTIFYITEMDRAW;
    }
    if (draw->dwDrawStage != CDDS_ITEMPREPAINT) return CDRF_DODEFAULT;

    RECT box = draw->rc;
    int column = (int)draw->dwItemSpec;
    fill(draw->hdc, box, colors.header);
    RECT bottomLine = {box.left, box.bottom - 1, box.right, box.bottom};
    fill(draw->hdc, bottomLine, colors.border);
    if (column < viewColumnCount() - 1) {
        RECT separator = {box.right - 1, box.top + S(5), box.right, box.bottom - S(5)};
        fill(draw->hdc, separator, colors.border);
    }

    string caption = "";
    if (column < numberColumns())
        caption = "№";
    else if (column < viewColumnCount())
        caption = headers[column - numberColumns()];
    RECT textBox = {box.left + S(8), box.top, box.right - S(4), box.bottom};
    drawText(draw->hdc, caption, textBox, fonts.hintBold, colors.muted, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    return CDRF_SKIPDEFAULT;  // стандартный заголовок не рисовать
}

// Надпись в пустой таблице результатов.
void Grid::paintPlaceholder() {
    if (editable || rowCount() > 0 || placeholder == "") return;
    HDC canvas = GetDC(hwnd);
    RECT box, header;
    GetClientRect(hwnd, &box);
    GetWindowRect(ListView_GetHeader(hwnd), &header);
    box.top += (header.bottom - header.top) + S(14);
    box.left += S(12);
    box.right -= S(12);
    drawText(canvas, placeholder, box, fonts.hint, colors.muted, DT_CENTER | DT_WORDBREAK);
    ReleaseDC(hwnd, canvas);
}
