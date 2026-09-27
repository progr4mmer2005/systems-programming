// Шрифты окна.
#pragma once

#include "../platform/system_dll.h"
#include "metrics.h"

struct Fonts {
    HFONT normal;    // обычный текст
    HFONT bold;      // кнопки, подписи
    HFONT hint;      // подсказки
    HFONT hintBold;  // заголовки столбцов
    HFONT title;     // заголовок окна
    HFONT caption;   // заголовки таблиц
    HFONT mono;      // таблицы с кодом (все буквы одной ширины)
};

Fonts fonts = {};

// size - размер в десятых долях пункта: 95 = 9.5 pt.
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
}
