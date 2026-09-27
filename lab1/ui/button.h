// Кнопки со своим оформлением: скруглённые, подсвечиваются при наведении.
#pragma once

#include <string>
#include <vector>

using namespace std;

#include "../util/text.h"
#include "draw.h"
#include "fonts.h"
#include "theme.h"

const int BUTTON_PRIMARY = 0;    // синяя
const int BUTTON_SECONDARY = 1;  // серая

struct ButtonInfo {
    HWND hwnd;
    int kind;         // BUTTON_PRIMARY или BUTTON_SECONDARY
    bool hover;       // мышь над кнопкой
    WNDPROC oldProc;  // стандартный обработчик кнопки Windows
};

vector<ButtonInfo> buttons;

int findButton(HWND hwnd) {
    for (int i = 0; i < len(buttons); i++) {
        if (buttons[i].hwnd == hwnd) return i;
    }
    return -1;
}

// Обработчик сообщений кнопки: следим, когда мышь заходит на кнопку и уходит с неё.
LRESULT CALLBACK buttonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    int i = findButton(hwnd);
    if (message == WM_MOUSEMOVE && !buttons[i].hover) {
        buttons[i].hover = true;
        TRACKMOUSEEVENT track = {sizeof(track), TME_LEAVE, hwnd, 0};  // попросить сообщить, когда мышь уйдёт
        TrackMouseEvent(&track);
        InvalidateRect(hwnd, NULL, FALSE);
    }
    if (message == WM_MOUSELEAVE) {
        buttons[i].hover = false;
        InvalidateRect(hwnd, NULL, FALSE);
    }
    if (message == WM_ERASEBKGND) return 1;  // фон рисуем сами
    return CallWindowProcW(buttons[i].oldProc, hwnd, message, wParam, lParam);
}

HWND makeButton(HWND parent, int id, string caption, int kind) {
    HWND hwnd = CreateWindowExW(0, L"BUTTON", toWide(caption).c_str(), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW, 0,
                                0, 10, 10, parent, (HMENU)(INT_PTR)id, GetModuleHandleW(NULL), NULL);
    ButtonInfo info = {hwnd, kind, false, NULL};
    info.oldProc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)buttonProc);
    buttons.push_back(info);
    return hwnd;
}

// Нарисовать кнопку. Windows просит об этом сообщением WM_DRAWITEM.
void drawButton(DRAWITEMSTRUCT* item) {
    int i = findButton(item->hwndItem);
    bool disabled = item->itemState & ODS_DISABLED;
    bool pressed = item->itemState & ODS_SELECTED;
    bool hover = i != -1 && buttons[i].hover;
    int kind = i != -1 ? buttons[i].kind : BUTTON_SECONDARY;

    COLORREF background, textColor;
    if (disabled) {
        background = colors.disabled;
        textColor = colors.disabledInk;
    } else if (kind == BUTTON_PRIMARY) {
        background = pressed ? colors.accentPressed : hover ? colors.accentHover : colors.accent;
        textColor = colors.onAccent;
    } else {
        background = (pressed || hover) ? colors.secondaryHover : colors.secondary;
        textColor = colors.ink;
    }
    fill(item->hDC, item->rcItem, colors.window);
    roundRect(item->hDC, item->rcItem, S(10), background, background);
    drawText(item->hDC, windowText(item->hwndItem), item->rcItem, fonts.bold, textColor, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}
