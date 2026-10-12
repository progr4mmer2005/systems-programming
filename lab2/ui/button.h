#pragma once

#include <string>
#include <vector>

using namespace std;

#include "../util/text.h"
#include "draw.h"
#include "fonts.h"
#include "theme.h"

const int BUTTON_PRIMARY = 0;
const int BUTTON_SECONDARY = 1;
const int BUTTON_ICON = 2;

struct ButtonInfo {
    HWND hwnd;
    int kind;
    bool hover;
    WNDPROC oldProc;
};

vector<ButtonInfo> buttons;

int findButton(HWND hwnd) {
    for (int i = 0; i < len(buttons); i++) {
        if (buttons[i].hwnd == hwnd) return i;
    }
    return -1;
}

LRESULT CALLBACK buttonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    int i = findButton(hwnd);
    if (message == WM_MOUSEMOVE && !buttons[i].hover) {
        buttons[i].hover = true;
        TRACKMOUSEEVENT track = {sizeof(track), TME_LEAVE, hwnd, 0};
        TrackMouseEvent(&track);
        InvalidateRect(hwnd, NULL, FALSE);
    }
    if (message == WM_MOUSELEAVE) {
        buttons[i].hover = false;
        InvalidateRect(hwnd, NULL, FALSE);
    }
    if (message == WM_ERASEBKGND) return 1;
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
    } else if (kind == BUTTON_ICON) {
        background = pressed ? colors.secondaryHover : hover ? colors.secondary : colors.window;
        textColor = (pressed || hover) ? colors.ink : colors.muted;
    } else {
        background = (pressed || hover) ? colors.secondaryHover : colors.secondary;
        textColor = colors.ink;
    }
    fill(item->hDC, item->rcItem, colors.window);
    roundRect(item->hDC, item->rcItem, S(10), background, background);
    HFONT font = kind == BUTTON_ICON ? fonts.icon : fonts.bold;
    drawText(item->hDC, windowText(item->hwndItem), item->rcItem, font, textColor, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

HWND tooltip = NULL;
vector<HWND> tooltipControls;

void setTooltip(HWND control, string text) {
    if (!tooltip) {
        tooltip = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX, CW_USEDEFAULT,
                                  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, GetParent(control), NULL, GetModuleHandleW(NULL), NULL);
    }
    wstring wide = toWide(text);
    TOOLINFOW tool = {};
    tool.cbSize = sizeof(tool);
    tool.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    tool.hwnd = GetParent(control);
    tool.uId = (UINT_PTR)control;
    tool.lpszText = (LPWSTR)wide.c_str();
    bool added = false;
    for (HWND other : tooltipControls) added = added || other == control;
    if (added) {
        SendMessageW(tooltip, TTM_UPDATETIPTEXTW, 0, (LPARAM)&tool);
    } else {
        SendMessageW(tooltip, TTM_ADDTOOLW, 0, (LPARAM)&tool);
        tooltipControls.push_back(control);
    }
}
