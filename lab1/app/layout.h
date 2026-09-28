// Раскладка окна: где стоит каждый элемент. Пересчитывается при изменении размера окна.
//
//   ┌ заголовок лабы ─────────────────── этапы 1 → 2 → 3 ── [Тёмная тема] ┐
//   │ ┌ входные данные ┐  ┌ первый проход ┐  ┌ второй проход ┐          │
//   │ └────────────────┘  └───────────────┘  └───────────────┘          │
//   └ [Первый проход] [Второй проход] [Очистить]  статус    Пример: [▼]  ┘
#pragma once

#include <algorithm>
#include <string>
#include <vector>

using namespace std;

#include "state.h"

// Таблица на карточке: заголовок, подсказка справа и доля высоты.
struct Slot {
    Grid* grid;
    string caption;
    string hint;
    int share;        // доля свободной высоты
    int fixedHeight;  // или точная высота (если больше 0)
};

// Разложить таблицы сверху вниз внутри area.
void stackGrids(RECT area, vector<Slot> slots) {
    int titleHeight = S(28);
    int gap = S(12);
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
            if (shareUsed == shareTotal) height = heightLeft;  // последняя добирает остаток
            heightLeft -= height;
        }
        height = max(height, S(40));

        Section section;
        section.title = {area.left, y, area.right, y + titleHeight};
        section.box = {area.left + 1, y + titleHeight, area.right - 1, y + titleHeight + height};
        section.caption = slot.caption;
        section.hint = slot.hint;
        sections.push_back(section);
        MoveWindow(slot.grid->hwnd, section.box.left, section.box.top, section.box.right - section.box.left, height, TRUE);
        y += titleHeight + height + gap;
    }
}

// Отступ внутри карточки.
RECT inside(RECT card) {
    int padding = S(16);
    return {card.left + padding, card.top + padding - S(4), card.right - padding, card.bottom - padding};
}

void layout() {
    RECT client;
    GetClientRect(mainWindow, &client);
    int width = client.right, height = client.bottom;
    int margin = S(18), gap = S(14), topHeight = S(74), bottomHeight = S(68);

    // Три карточки: 36% / 31% / остаток ширины.
    int free = width - 2 * margin - 2 * gap;
    RECT left = {margin, topHeight, margin + free * 36 / 100, height - bottomHeight};
    RECT middle = {left.right + gap, topHeight, left.right + gap + free * 31 / 100, height - bottomHeight};
    RECT right = {middle.right + gap, topHeight, width - margin, height - bottomHeight};
    cards = {left, middle, right};
    sections.clear();

    // Левая карточка: две таблицы, под ними поле адреса и подсказка по клавишам.
    RECT area = inside(left);
    int addressHeight = S(30), hintHeight = S(18);
    RECT gridsArea = area;
    gridsArea.bottom -= addressHeight + hintHeight + S(14);
    stackGrids(gridsArea, {{&sourceGrid, "Исходный текст", "правый клик — вставить / удалить строку", 62, 0},
                           {&opsGrid, "Таблица кодов операций (ТКО)", "правый клик — меню", 38, 0}});
    int y = gridsArea.bottom + S(10);
    addressLabelBox = {area.left, y, area.left + S(190), y + addressHeight};
    addressBox = {addressLabelBox.right, y, addressLabelBox.right + S(140), y + addressHeight};
    int editHeight = S(18);  // само поле - ровно посередине рамки
    MoveWindow(addressEdit, addressBox.left + S(10), y + (addressHeight - editHeight) / 2, S(120), editHeight, TRUE);
    keysHintBox = {area.left, y + addressHeight + S(4), area.right, y + addressHeight + S(4) + hintHeight};

    // Средняя и правая карточки.
    stackGrids(inside(middle), {{&supportGrid, "Вспомогательная таблица", "МКОП = код * 4 + адресация", 46, 0},
                                {&symbolsGrid, "Таблица символических имён (ТСИ)", "метка → адрес", 30, 0},
                                {&errors1Grid, "Ошибки первого прохода", "", 24, 0}});
    // Заголовок объектного модуля - первая строка двоичного кода (запись H).
    stackGrids(inside(right), {{&codeGrid, "Двоичный код", "H - заголовок, T - код, E - конец", 78, 0},
                               {&errors2Grid, "Ошибки второго прохода", "", 22, 0}});

    // Нижняя полоса: кнопки, строка состояния, список примеров.
    int buttonY = height - bottomHeight + S(16), buttonHeight = S(38), buttonWidth = S(190);
    MoveWindow(pass1Button, margin, buttonY, buttonWidth, buttonHeight, TRUE);
    MoveWindow(pass2Button, margin + S(200), buttonY, buttonWidth, buttonHeight, TRUE);
    MoveWindow(resetButton, margin + S(400), buttonY, buttonWidth, buttonHeight, TRUE);
    int listWidth = S(340);
    MoveWindow(examplesList, width - margin - listWidth, buttonY + S(5), listWidth, S(400), TRUE);
    exampleLabelBox = {width - margin - listWidth - S(80), buttonY, width - margin - listWidth - S(10), buttonY + buttonHeight};
    statusBox = {margin + S(610), buttonY - S(4), exampleLabelBox.left - S(16), buttonY + buttonHeight + S(4)};

    // Верхняя полоса: кнопка темы и этапы.
    int themeWidth = S(150);
    MoveWindow(themeButton, width - margin - themeWidth, S(24), themeWidth, S(34), TRUE);
    stepsBox = {width / 2, 0, width - margin - themeWidth - S(24), topHeight};

    for (Grid* grid : allGrids) grid->fitColumns();

    // Размер таблиц задаётся только здесь, поэтому при первой раскладке
    // их нужно прокрутить к первой строке.
    static bool firstLayout = true;
    if (firstLayout && width > 0) {
        firstLayout = false;
        for (Grid* grid : allGrids) grid->scrollToTop();
    }
    InvalidateRect(mainWindow, NULL, FALSE);
}
