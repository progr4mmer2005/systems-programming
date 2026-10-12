#pragma once

#include "../platform/system_dll.h"
#include "metrics.h"

struct Fonts {
    HFONT normal;
    HFONT bold;
    HFONT hint;
    HFONT hintBold;
    HFONT title;
    HFONT caption;
    HFONT mono;
    HFONT icon;
};

Fonts fonts = {};

HFONT makeFont(int size, int weight, const wchar_t* name) {
    int height = -MulDiv(size, screenDpi, 720);
    return gdi.CreateFontW(height, 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                           CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, name);
}

void createFonts() {
    fonts.normal = makeFont(95, FW_NORMAL, L"Segoe UI");
    fonts.bold = makeFont(95, FW_SEMIBOLD, L"Segoe UI");
    fonts.hint = makeFont(85, FW_NORMAL, L"Segoe UI");
    fonts.hintBold = makeFont(85, FW_SEMIBOLD, L"Segoe UI");
    fonts.title = makeFont(160, FW_SEMIBOLD, L"Segoe UI");
    fonts.caption = makeFont(105, FW_SEMIBOLD, L"Segoe UI");
    fonts.mono = makeFont(100, FW_NORMAL, L"Consolas");
    fonts.icon = makeFont(110, FW_NORMAL, L"Segoe MDL2 Assets");
}
