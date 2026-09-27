// Простые функции рисования.
//
#pragma once

#include <string>

using namespace std;

#include "../platform/system_dll.h"
#include "../platform/unicode.h"

// Залить прямоугольник цветом.
void fill(HDC canvas, RECT box, COLORREF color) {
    HBRUSH brush = gdi.CreateSolidBrush(color);
    FillRect(canvas, &box, brush);
    gdi.DeleteObject(brush);
}

// Рамка толщиной 1 пиксель.
void frame(HDC canvas, RECT box, COLORREF color) {
    HBRUSH brush = gdi.CreateSolidBrush(color);
    FrameRect(canvas, &box, brush);
    gdi.DeleteObject(brush);
}

// Прямоугольник со скруглёнными углами.
void roundRect(HDC canvas, RECT box, int radius, COLORREF fillColor, COLORREF borderColor) {
    HBRUSH brush = gdi.CreateSolidBrush(fillColor);
    HPEN pen = gdi.CreatePen(PS_SOLID, 1, borderColor);
    HGDIOBJ oldBrush = gdi.SelectObject(canvas, brush);
    HGDIOBJ oldPen = gdi.SelectObject(canvas, pen);
    gdi.RoundRect(canvas, box.left, box.top, box.right, box.bottom, radius, radius);
    gdi.SelectObject(canvas, oldBrush);
    gdi.SelectObject(canvas, oldPen);
    gdi.DeleteObject(brush);
    gdi.DeleteObject(pen);
}

void circle(HDC canvas, RECT box, COLORREF color) {
    HBRUSH brush = gdi.CreateSolidBrush(color);
    HPEN pen = gdi.CreatePen(PS_SOLID, 1, color);
    HGDIOBJ oldBrush = gdi.SelectObject(canvas, brush);
    HGDIOBJ oldPen = gdi.SelectObject(canvas, pen);
    gdi.Ellipse(canvas, box.left, box.top, box.right, box.bottom);
    gdi.SelectObject(canvas, oldBrush);
    gdi.SelectObject(canvas, oldPen);
    gdi.DeleteObject(brush);
    gdi.DeleteObject(pen);
}

// Текст в прямоугольнике. align - выравнивание, например DT_LEFT | DT_VCENTER | DT_SINGLELINE.
void drawText(HDC canvas, string text, RECT box, HFONT font, COLORREF color, UINT align) {
    HGDIOBJ oldFont = gdi.SelectObject(canvas, font);
    gdi.SetTextColor(canvas, color);
    gdi.SetBkMode(canvas, TRANSPARENT);
    wstring wide = toWide(text);
    DrawTextW(canvas, wide.c_str(), (int)wide.size(), &box, align | DT_NOPREFIX);
    gdi.SelectObject(canvas, oldFont);
}
