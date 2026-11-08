#pragma once

#include "grid.h"

LRESULT Grid::paintCells(NMLVCUSTOMDRAW* draw) {
    int stage = draw->nmcd.dwDrawStage;
    int row = (int)draw->nmcd.dwItemSpec;

    draw->nmcd.uItemState &= ~(CDIS_SELECTED | CDIS_FOCUS | CDIS_HOT);

    if (stage == CDDS_PREPAINT) return CDRF_NOTIFYITEMDRAW;

    if (stage == CDDS_ITEMPREPAINT) return CDRF_NOTIFYSUBITEMDRAW | CDRF_NOTIFYPOSTPAINT;

    if (stage == CDDS_ITEMPOSTPAINT) {
        drawCellLines(draw->nmcd.hdc, row);
        return CDRF_DODEFAULT;
    }

    if (stage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM)) {
        if (draw->iSubItem < numberColumns()) {
            draw->clrText = colors.muted;
            draw->clrTextBk = colors.numberColumn;
            return CDRF_NEWFONT;
        }
        int col = draw->iSubItem - numberColumns();
        COLORREF background = colors.field;
        COLORREF text = colors.ink;

        CellColors custom = cellColors(id, row, col);
        if (custom.hasBackground) background = custom.background;
        if (custom.hasText) text = custom.text;

        if (row == currentRow) {
            bool focused = GetFocus() == hwnd;
            bool isCurrentCell = editable && col == currentCol && focused;
            if (focused && !isCurrentCell) {
                background = colors.accent;
                text = colors.onAccent;
            } else if (!focused && !custom.hasBackground) {
                background = colors.numberColumn;
            }
        }
        draw->clrText = text;
        draw->clrTextBk = background;
        return CDRF_NEWFONT;
    }
    return CDRF_DODEFAULT;
}

void Grid::drawCellLines(HDC canvas, int row) {
    if (!editable || row != currentRow || GetFocus() != hwnd) return;
    RECT box = cellBox(row, currentCol);
    frame(canvas, box, colors.accent);
    InflateRect(&box, -1, -1);
    frame(canvas, box, colors.accent);
}

void Grid::paintPlaceholder() {
    if (editable || rowCount() > 0 || placeholder == "") return;
    HDC canvas = GetDC(hwnd);
    RECT box, header;
    GetClientRect(hwnd, &box);
    GetWindowRect(ListView_GetHeader(hwnd), &header);
    box.top += header.bottom - header.top;
    box.left += S(8);
    box.right -= S(8);
    if (box.bottom - box.top < S(60)) {
        drawText(canvas, placeholder, box, fonts.normal, colors.muted, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    } else {
        box.top += S(12);
        drawText(canvas, placeholder, box, fonts.normal, colors.muted, DT_CENTER | DT_WORDBREAK);
    }
    ReleaseDC(hwnd, canvas);
}
