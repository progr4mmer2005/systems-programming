#pragma once

#include "../platform/system_dll.h"
#include "metrics.h"

struct Fonts {
    HFONT normal;
    HFONT mono;
};

Fonts fonts = {};

HFONT makeFont(int size, int weight, const wchar_t* name) {
    int height = -MulDiv(size, screenDpi, 720);
    return gdi.CreateFontW(height, 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                           CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, name);
}

void createFonts() {
    fonts.normal = makeFont(90, FW_NORMAL, L"Segoe UI");
    fonts.mono = makeFont(100, FW_NORMAL, L"Consolas");
}
