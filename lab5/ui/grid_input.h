#pragma once

#include "grid.h"

bool Grid::onKeyDown(WPARAM key) {
    bool ctrl = GetKeyState(VK_CONTROL) < 0;
    bool shift = GetKeyState(VK_SHIFT) < 0;
    if (key == VK_RETURN || key == VK_F2) beginEdit(L"");
    else if (key == VK_LEFT) moveColumn(-1);
    else if (key == VK_RIGHT) moveColumn(+1);
    else if (key == VK_TAB) moveColumn(shift ? -1 : +1);
    else if (key == VK_INSERT) insertRow(currentRow);
    else if (key == VK_DELETE && ctrl) deleteRow(currentRow);
    else if (key == VK_DELETE) clearCell();
    else if (key == 'E' && ctrl) editAsText();
    else return false;
    return true;
}

LRESULT CALLBACK gridProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    Grid* grid = findGrid(hwnd);

    if (message == WM_NOTIFY) {
        NMHDR* note = (NMHDR*)lParam;
        if (note->hwndFrom == ListView_GetHeader(hwnd)) {
            if (note->code == (UINT)HDN_BEGINTRACKW || note->code == (UINT)HDN_DIVIDERDBLCLICKW) return TRUE;
        }
    }

    if (message == WM_SIZE) {
        LRESULT result = CallWindowProcW(grid->oldProc, hwnd, message, wParam, lParam);
        grid->fitColumns();
        return result;
    }

    if (message == WM_PAINT) {
        LRESULT result = CallWindowProcW(grid->oldProc, hwnd, message, wParam, lParam);
        grid->paintPlaceholder();
        return result;
    }

    if (message == WM_SETFOCUS || message == WM_KILLFOCUS) InvalidateRect(hwnd, NULL, FALSE);

    if (message == WM_VSCROLL || message == WM_HSCROLL || message == WM_MOUSEWHEEL) {
        grid->finishEdit(true, 0, 0, true);
    }


    if (message == WM_LBUTTONDOWN) {
        grid->finishEdit(true, 0, 0, false);
        LVHITTESTINFO hit = {};
        hit.pt = {mouseX(lParam), mouseY(lParam)};
        ListView_SubItemHitTest(hwnd, &hit);
        if (hit.iItem >= 0) grid->currentCol = grid->toDataColumn(hit.iSubItem);
        LRESULT result = CallWindowProcW(grid->oldProc, hwnd, message, wParam, lParam);
        SetFocus(hwnd);
        if (hit.iItem >= 0) {
            grid->currentRow = hit.iItem;
            InvalidateRect(hwnd, NULL, FALSE);
            onGridRowClicked(grid->id, hit.iItem);
        }
        return result;
    }

    if (message == WM_LBUTTONDBLCLK && grid->editable) {
        LVHITTESTINFO hit = {};
        hit.pt = {mouseX(lParam), mouseY(lParam)};
        ListView_SubItemHitTest(hwnd, &hit);
        if (hit.iItem >= 0) {
            grid->currentRow = hit.iItem;
            grid->currentCol = grid->toDataColumn(hit.iSubItem);
            grid->beginEdit(L"");
        }
        return 0;
    }

    if (message == WM_KEYDOWN) {
        if (grid->editable && grid->onKeyDown(wParam)) return 0;
        LRESULT result = CallWindowProcW(grid->oldProc, hwnd, message, wParam, lParam);
        int focused = ListView_GetNextItem(hwnd, -1, LVNI_FOCUSED);
        if (focused >= 0 && focused != grid->currentRow) {
            grid->currentRow = focused;
            InvalidateRect(hwnd, NULL, FALSE);
            onGridRowClicked(grid->id, focused);
        }
        return result;
    }

    if (message == WM_CHAR && grid->editable) {
        bool ctrl = GetKeyState(VK_CONTROL) < 0;
        if (wParam >= 32 && !ctrl) grid->beginEdit(wstring(1, (wchar_t)wParam));
        return 0;
    }

    if (message == WM_GETDLGCODE) return DLGC_WANTALLKEYS;

    if (message == WM_CONTEXTMENU) {
        grid->showContextMenu(lParam);
        return 0;
    }

    return CallWindowProcW(grid->oldProc, hwnd, message, wParam, lParam);
}

LRESULT CALLBACK cellEditorProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    Grid* grid = findGrid(GetParent(hwnd));
    WNDPROC oldEditorProc = grid->oldEditorProc;

    if (message == WM_GETDLGCODE) return DLGC_WANTALLKEYS;
    if (message == WM_KEYDOWN) {
        bool shift = GetKeyState(VK_SHIFT) < 0;
        if (wParam == VK_RETURN) { grid->finishEdit(true, +1, 0, true); return 0; }
        if (wParam == VK_ESCAPE) { grid->finishEdit(false, 0, 0, true); return 0; }
        if (wParam == VK_TAB) { grid->finishEdit(true, 0, shift ? -1 : +1, true); return 0; }
        if (wParam == VK_UP) { grid->finishEdit(true, -1, 0, true); return 0; }
        if (wParam == VK_DOWN) { grid->finishEdit(true, +1, 0, true); return 0; }
    }
    if (message == WM_CHAR && (wParam == VK_RETURN || wParam == VK_ESCAPE || wParam == VK_TAB)) return 0;
    if (message == WM_KILLFOCUS) {
        grid->finishEdit(true, 0, 0, false);
        return 0;
    }
    return CallWindowProcW(oldEditorProc, hwnd, message, wParam, lParam);
}
