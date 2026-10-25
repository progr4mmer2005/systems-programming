#pragma once

#include <algorithm>
#include <string>
#include <vector>

using namespace std;

#include "state.h"

struct Slot {
    Grid* grid;
    string caption;
    int share;
    int fixedHeight;
};

const int MARGIN = 8;
const int GAP = 8;
const int TITLE_HEIGHT = 20;
const int FIELD_HEIGHT = 23;
const int BUTTON_HEIGHT = 26;

void stackGrids(RECT area, vector<Slot> slots) {
    int titleHeight = S(TITLE_HEIGHT), gap = S(GAP);
    int fixedTotal = 0, shareTotal = 0;
    for (Slot slot : slots) {
        fixedTotal += titleHeight + slot.fixedHeight;
        if (slot.fixedHeight == 0) shareTotal += slot.share;
    }
    int freeHeight = (area.bottom - area.top) - fixedTotal - gap * (len(slots) - 1);

    int y = area.top;
    int shareUsed = 0, heightLeft = freeHeight;
    for (Slot slot : slots) {
        int height = slot.fixedHeight;
        if (height == 0) {
            shareUsed += slot.share;
            height = freeHeight * slot.share / shareTotal;
            if (shareUsed == shareTotal) height = heightLeft;
            heightLeft -= height;
        }
        height = max(height, S(40));
        sections.push_back({{area.left, y, area.right, y + titleHeight - S(3)}, slot.caption});
        MoveWindow(slot.grid->hwnd, area.left, y + titleHeight, area.right - area.left, height, TRUE);
        y += titleHeight + height + gap;
    }
}

void layout() {
    RECT client, statusBox;
    GetClientRect(mainWindow, &client);
    SendMessageW(statusBar, WM_SIZE, 0, 0);
    GetWindowRect(statusBar, &statusBox);
    int width = client.right, height = client.bottom - (statusBox.bottom - statusBox.top);
    int margin = S(MARGIN), gap = S(GAP), fieldHeight = S(FIELD_HEIGHT), buttonHeight = S(BUTTON_HEIGHT);
    int top = margin, bottom = height - buttonHeight - 2 * margin;

    int free = width - 2 * margin - 2 * gap;
    RECT left = {margin, top, margin + free * 34 / 100, bottom};
    RECT middle = {left.right + gap, top, left.right + gap + free * 33 / 100, bottom};
    RECT right = {middle.right + gap, top, width - margin, bottom};
    sections.clear();

    RECT gridsArea = left;
    gridsArea.bottom -= fieldHeight + gap;
    stackGrids(gridsArea, {{&sourceGrid, "Исходный текст", 62, 0}, {&opsGrid, "Таблица кодов операций", 38, 0}});
    int y = gridsArea.bottom + gap;
    addressLabelBox = {left.left, y, left.left + S(105), y + fieldHeight};
    MoveWindow(addressEdit, addressLabelBox.right, y, S(100), fieldHeight, TRUE);

    stackGrids(middle, {{&symbolsGrid, "Таблица символических имён", 60, 0}, {&errorsGrid, "Ошибки", 40, 0}});
    stackGrids(right, {{&headerGrid, "Заголовок объектного модуля", 0, S(52)}, {&codeGrid, "Двоичный код", 1, 0}});

    int buttonY = height - margin - buttonHeight, buttonWidth = S(130);
    MoveWindow(stepButton, margin, buttonY, buttonWidth, buttonHeight, TRUE);
    MoveWindow(runButton, margin + buttonWidth + gap, buttonY, buttonWidth, buttonHeight, TRUE);
    MoveWindow(resetButton, margin + 2 * (buttonWidth + gap), buttonY, buttonWidth, buttonHeight, TRUE);
    int listWidth = S(320);
    MoveWindow(examplesList, width - margin - listWidth, buttonY + (buttonHeight - fieldHeight) / 2, listWidth, S(300), TRUE);
    exampleLabelBox = {width - margin - listWidth - S(70), buttonY, width - margin - listWidth - S(6), buttonY + buttonHeight};

    for (Grid* grid : allGrids) grid->fitColumns();

    static bool firstLayout = true;
    if (firstLayout && width > 0) {
        firstLayout = false;
        for (Grid* grid : allGrids) grid->scrollToTop();
    }
    InvalidateRect(mainWindow, NULL, FALSE);
}
