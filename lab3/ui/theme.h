// Цвета окна: светлая и тёмная тема.
#pragma once

#include "../platform/system_dll.h"

struct Palette {
    COLORREF window, card, border, grid, ink, muted;
    COLORREF accent, accentHover, accentPressed, onAccent;
    COLORREF secondary, secondaryHover, disabled, disabledInk;
    COLORREF header, zebra, rowSelected, cellSelected;
    COLORREF error, errorBackground, ok;
};

const Palette LIGHT = {
    RGB(238, 242, 247), RGB(255, 255, 255), RGB(210, 218, 229), RGB(214, 221, 231), RGB(15, 23, 42), RGB(100, 116, 139),
    RGB(37, 99, 235), RGB(59, 130, 246), RGB(29, 78, 216), RGB(255, 255, 255),
    RGB(226, 232, 240), RGB(203, 213, 225), RGB(214, 221, 230), RGB(255, 255, 255),
    RGB(243, 246, 250), RGB(249, 250, 252), RGB(232, 240, 254), RGB(210, 227, 252),
    RGB(185, 28, 28), RGB(254, 226, 226), RGB(22, 163, 74),
};

const Palette DARK = {
    RGB(15, 17, 21), RGB(24, 27, 33), RGB(52, 58, 68), RGB(50, 56, 66), RGB(229, 231, 235), RGB(148, 158, 173),
    RGB(59, 130, 246), RGB(96, 165, 250), RGB(37, 99, 235), RGB(255, 255, 255),
    RGB(45, 51, 61), RGB(60, 67, 79), RGB(40, 45, 54), RGB(107, 114, 128),
    RGB(31, 35, 42), RGB(28, 31, 38), RGB(30, 45, 74), RGB(37, 70, 128),
    RGB(248, 113, 113), RGB(76, 29, 34), RGB(74, 222, 128),
};

Palette colors = LIGHT;      // текущие цвета - всё рисуется ими
bool darkTheme = false;
HBRUSH inputBrush = NULL;    // кисть для фона полей ввода

void setDarkTheme(bool dark) {
    darkTheme = dark;
    colors = dark ? DARK : LIGHT;
    if (inputBrush) gdi.DeleteObject(inputBrush);
    inputBrush = gdi.CreateSolidBrush(colors.card);
}

// Тёмный вид системного элемента (полосы прокрутки, выпадающий список).
void applyControlTheme(HWND control, const wchar_t* darkThemeName) {
    if (themeApi.SetWindowTheme) themeApi.SetWindowTheme(control, darkTheme ? darkThemeName : NULL, NULL);
}

// Тёмный заголовок окна (Windows 10 и новее). 20 - номер этой настройки в Windows.
void applyTitleBarTheme(HWND window) {
    if (!themeApi.DwmSetWindowAttribute) return;
    BOOL on = darkTheme ? TRUE : FALSE;
    themeApi.DwmSetWindowAttribute(window, 20, &on, sizeof(on));
}
