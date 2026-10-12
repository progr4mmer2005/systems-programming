#pragma once

#include <string>

using namespace std;

#include "../config.h"
#include "state.h"

void drawSteps(HDC canvas) {
    vector<string> labels = {"Исходные данные", "Первый проход", "Второй проход"};
    vector<int> states = {STEP_OK, step1, step2};
    int stepWidth = S(170), size = S(24), y = S(26);
    int x = stepsBox.right - 3 * stepWidth;

    for (int i = 0; i < 3; i++) {
        int left = x + i * stepWidth;
        int state = states[i];
        if (i > 0) {
            RECT line = {left - stepWidth + size + S(118), y + size / 2, left - S(8), y + size / 2 + S(2)};
            fill(canvas, line, states[i - 1] == STEP_OK ? colors.ok : colors.border);
        }
        RECT dot = {left, y, left + size, y + size};
        COLORREF dotColor = colors.secondaryHover;
        if (state == STEP_OK) dotColor = colors.ok;
        if (state == STEP_FAILED) dotColor = colors.error;
        circle(canvas, dot, dotColor);

        string mark = state == STEP_FAILED ? "!" : to_string(i + 1);
        COLORREF markColor = state == STEP_WAITING ? colors.muted : colors.onAccent;
        drawText(canvas, mark, dot, fonts.hintBold, markColor, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        RECT labelBox = {left + size + S(8), y, left + stepWidth, y + size};
        COLORREF labelColor = state == STEP_WAITING ? colors.muted : colors.ink;
        drawText(canvas, labels[i], labelBox, fonts.hint, labelColor, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }
}

void paintWindow(HDC canvas, RECT client) {
    fill(canvas, client, colors.window);

    int margin = S(18);
    RECT smallTitle = {margin, S(12), client.right / 2, S(32)};
    drawText(canvas, LAB_TITLE, smallTitle, fonts.hint, colors.muted, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    RECT bigTitle = {margin, S(30), client.right / 2 + S(200), S(62)};
    drawText(canvas, LAB_SUBTITLE, bigTitle, fonts.title, colors.ink, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    drawSteps(canvas);

    for (RECT card : cards) roundRect(canvas, card, S(14), colors.card, colors.border);
    for (Section section : sections) {
        drawText(canvas, section.caption, section.title, fonts.caption, colors.ink, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        drawText(canvas, section.hint, section.title, fonts.hint, colors.muted, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        RECT border = section.box;
        InflateRect(&border, 1, 1);
        frame(canvas, border, colors.border);
    }

    drawText(canvas, "Адресация:", modeLabelBox, fonts.bold, colors.ink, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    drawText(canvas, "Двойной клик / Enter — изменить · правый клик — меню · Ins — строка выше · Ctrl+Del — удалить строку",
             keysHintBox, fonts.hint, colors.muted, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    drawText(canvas, "Пример:", exampleLabelBox, fonts.bold, colors.ink, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    drawText(canvas, statusText, statusBox, fonts.normal, statusColor, DT_LEFT | DT_VCENTER | DT_WORDBREAK | DT_END_ELLIPSIS);
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
