#pragma once

#include "grid.h"

bool plusHover = false;

LRESULT CALLBACK plusButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    Grid* grid = findGrid(GetParent(hwnd));
    if (message == WM_PAINT) {
        PAINTSTRUCT paint;
        HDC canvas = BeginPaint(hwnd, &paint);
        RECT box;
        GetClientRect(hwnd, &box);
        circle(canvas, box, plusHover ? colors.accentHover : colors.accent);
        int middle = box.right / 2, arm = box.right / 4, thick = max(S(2), 2);
        RECT horizontal = {middle - arm, middle - thick / 2, middle + arm + 1, middle - thick / 2 + thick};
        RECT vertical = {middle - thick / 2, middle - arm, middle - thick / 2 + thick, middle + arm + 1};
        fill(canvas, horizontal, colors.onAccent);
        fill(canvas, vertical, colors.onAccent);
        EndPaint(hwnd, &paint);
        return 0;
    }
    if (message == WM_ERASEBKGND) return 1;
    if (message == WM_SETCURSOR) {
        SetCursor(LoadCursorW(NULL, IDC_HAND));
        return TRUE;
    }
    if (message == WM_MOUSEMOVE && !plusHover) {
        plusHover = true;
        TRACKMOUSEEVENT track = {sizeof(track), TME_LEAVE, hwnd, 0};
        TrackMouseEvent(&track);
        InvalidateRect(hwnd, NULL, FALSE);
    }
    if (message == WM_MOUSELEAVE) {
        plusHover = false;
        InvalidateRect(hwnd, NULL, FALSE);
        if (grid) grid->hidePlusIfAway();
    }
    if (message == WM_LBUTTONDOWN) return 0;
    if (message == WM_LBUTTONUP && grid) {
        int at = grid->plusAt;
        grid->hidePlus();
        if (at >= 0) grid->insertRow(at);
        return 0;
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}

int Grid::boundaryY(int at) {
    RECT box;
    if (at < rowCount()) {
        if (!ListView_GetItemRect(hwnd, at, &box, LVIR_BOUNDS)) return -1;
        return box.top;
    }
    if (rowCount() == 0 || !ListView_GetItemRect(hwnd, rowCount() - 1, &box, LVIR_BOUNDS)) return -1;
    return box.bottom;
}

int Grid::boundaryAt(int y) {
    if (!editable || rowCount() == 0 || editor) return -1;
    RECT header, client;
    GetWindowRect(ListView_GetHeader(hwnd), &header);
    GetClientRect(hwnd, &client);
    int top = header.bottom - header.top;
    if (y < top || y > client.bottom) return -1;
    int first = ListView_GetTopIndex(hwnd);
    int last = min(rowCount(), first + ListView_GetCountPerPage(hwnd) + 1);
    if (keepEmptyLastRow) last = min(last, rowCount() - 1);
    for (int at = first; at <= last; at++) {
        int edge = boundaryY(at);
        if (edge >= top && abs(y - edge) <= S(4)) return at;
    }
    return -1;
}

void Grid::onMouseMove(int y) {
    if (!trackingMouse) {
        TRACKMOUSEEVENT track = {sizeof(track), TME_LEAVE, hwnd, 0};
        TrackMouseEvent(&track);
        trackingMouse = true;
    }
    int at = boundaryAt(y);
    if (at == hoverAt) return;
    hoverAt = at;
    KillTimer(hwnd, PLUS_TIMER);
    if (plusAt != at) hidePlus();
    if (at != -1) SetTimer(hwnd, PLUS_TIMER, PLUS_DELAY, NULL);
}

void Grid::showPlus() {
    KillTimer(hwnd, PLUS_TIMER);
    if (hoverAt == -1 || editor) return;
    int edge = boundaryY(hoverAt);
    if (edge < 0) return;

    static bool registered = false;
    if (!registered) {
        WNDCLASSEXW plusClass = {};
        plusClass.cbSize = sizeof(plusClass);
        plusClass.lpfnWndProc = plusButtonProc;
        plusClass.hInstance = GetModuleHandleW(NULL);
        plusClass.hCursor = LoadCursorW(NULL, IDC_HAND);
        plusClass.lpszClassName = L"AsmGridPlus";
        RegisterClassExW(&plusClass);
        registered = true;
    }

    int size = S(18);
    if (!plusButton) {
        plusButton = CreateWindowExW(0, L"AsmGridPlus", L"", WS_CHILD, 0, 0, size, size, hwnd, NULL, GetModuleHandleW(NULL), NULL);
        SetWindowRgn(plusButton, gdi.CreateEllipticRgn(0, 0, size + 1, size + 1), TRUE);
    }

    RECT header;
    GetWindowRect(ListView_GetHeader(hwnd), &header);
    int x = showRowNumbers ? S(NUMBER_COLUMN_WIDTH) - size - S(3) : S(2);
    int y = max(edge - size / 2, (int)(header.bottom - header.top));
    plusAt = hoverAt;
    SetWindowPos(plusButton, HWND_TOP, x, y, size, size, SWP_SHOWWINDOW | SWP_NOACTIVATE);
    InvalidateRect(plusButton, NULL, FALSE);
}

void Grid::hidePlus() {
    plusAt = -1;
    if (plusButton) ShowWindow(plusButton, SW_HIDE);
}

void Grid::hidePlusIfAway() {
    POINT cursor;
    GetCursorPos(&cursor);
    if (plusButton && WindowFromPoint(cursor) == plusButton) return;
    KillTimer(hwnd, PLUS_TIMER);
    hoverAt = -1;
    hidePlus();
}
