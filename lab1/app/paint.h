#pragma once

#include <string>

using namespace std;

#include "state.h"

void paintWindow(HDC canvas, RECT client) {
    fill(canvas, client, colors.window);
    for (Section section : sections)
        drawText(canvas, section.caption, section.title, fonts.normal, colors.ink, DT_LEFT | DT_BOTTOM | DT_SINGLELINE | DT_END_ELLIPSIS);
    drawText(canvas, "Адрес загрузки:", addressLabelBox, fonts.normal, colors.ink, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    drawText(canvas, "Пример:", exampleLabelBox, fonts.normal, colors.ink, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
}

void paintWithoutFlicker(HWND window) {
    PAINTSTRUCT paint;
    HDC screen = BeginPaint(window, &paint);
    RECT client;
    GetClientRect(window, &client);
    HDC memory = gdi.CreateCompatibleDC(screen);
    HBITMAP picture = gdi.CreateCompatibleBitmap(screen, client.right, client.bottom);
    HGDIOBJ oldPicture = gdi.SelectObject(memory, picture);

    paintWindow(memory, client);
    gdi.BitBlt(screen, 0, 0, client.right, client.bottom, memory, 0, 0, SRCCOPY);

    gdi.SelectObject(memory, oldPicture);
    gdi.DeleteObject(picture);
    gdi.DeleteDC(memory);
    EndPaint(window, &paint);
}
