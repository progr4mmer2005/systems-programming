// Рисование окна: фон, заголовок, этапы, карточки, подписи таблиц, строка состояния.
// Таблицы и кнопки рисуют себя сами - здесь только то, что между ними.
#pragma once

#include <string>

using namespace std;

#include "../config.h"
#include "state.h"

// Этапы вверху окна: исходные данные, первый проход, второй проход.
void drawSteps(HDC canvas) {
    vector<string> labels = {"Исходные данные", "Первый проход", "Второй проход"};
    vector<int> states = {STEP_OK, step1, step2};
    int stepWidth = S(170), size = S(24), y = S(26);
    int x = stepsBox.right - 3 * stepWidth;

    for (int i = 0; i < 3; i++) {
        int left = x + i * stepWidth;
        int state = states[i];
        if (i > 0) {  // линия от предыдущего кружка
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

    // Заголовок.
    int margin = S(18);
    RECT smallTitle = {margin, S(12), client.right / 2, S(32)};
    drawText(canvas, LAB_TITLE, smallTitle, fonts.hint, colors.muted, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    RECT bigTitle = {margin, S(30), client.right / 2 + S(200), S(62)};
    drawText(canvas, LAB_SUBTITLE, bigTitle, fonts.title, colors.ink, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    drawSteps(canvas);

    // Карточки и подписи таблиц.
    for (RECT card : cards) roundRect(canvas, card, S(14), colors.card, colors.border);
    for (Section section : sections) {
        drawText(canvas, section.caption, section.title, fonts.caption, colors.ink, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        drawText(canvas, section.hint, section.title, fonts.hint, colors.muted, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        RECT border = section.box;
        InflateRect(&border, 1, 1);  // рамка на пиксель шире таблицы
        frame(canvas, border, colors.border);
    }

    // Поле адреса: подпись и скруглённая рамка (синяя, когда поле активно).
    drawText(canvas, "Адрес загрузки (hex):", addressLabelBox, fonts.bold, colors.ink, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    bool addressActive = GetFocus() == addressEdit;
    roundRect(canvas, addressBox, S(8), colors.card, addressActive ? colors.accent : colors.border);
    drawText(canvas, "Двойной клик / Enter — изменить · правый клик — меню · Ins — строка выше · Ctrl+Del — удалить строку",
             keysHintBox, fonts.hint, colors.muted, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    // Нижняя полоса.
    drawText(canvas, "Пример:", exampleLabelBox, fonts.bold, colors.ink, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    drawText(canvas, statusText, statusBox, fonts.normal, statusColor, DT_LEFT | DT_VCENTER | DT_WORDBREAK | DT_END_ELLIPSIS);
}

// Рисование в памяти и копирование на экран, чтобы окно не мерцало.
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
