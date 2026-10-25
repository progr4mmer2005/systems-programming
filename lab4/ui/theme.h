#pragma once

#include "../platform/system_dll.h"

struct Palette {
    COLORREF window, field, ink, muted, border;
    COLORREF accent, onAccent, numberColumn;
    COLORREF error, errorBackground;
    COLORREF current;
};

Palette colors = {};

void loadSystemColors() {
    colors.window = GetSysColor(COLOR_BTNFACE);
    colors.field = GetSysColor(COLOR_WINDOW);
    colors.ink = GetSysColor(COLOR_WINDOWTEXT);
    colors.muted = GetSysColor(COLOR_GRAYTEXT);
    colors.border = GetSysColor(COLOR_BTNSHADOW);
    colors.accent = GetSysColor(COLOR_HIGHLIGHT);
    colors.onAccent = GetSysColor(COLOR_HIGHLIGHTTEXT);
    colors.numberColumn = GetSysColor(COLOR_BTNFACE);
    colors.error = RGB(192, 0, 0);
    colors.errorBackground = RGB(255, 210, 210);
    colors.current = GetSysColor(COLOR_INFOBK);
}
