// Создание всех элементов окна: таблицы, кнопки, список примеров, поле адреса.
#pragma once

#include "../ui/button.h"
#include "actions.h"
#include "state.h"

void createGrids() {
    // Левая карточка - входные данные (можно редактировать).
    sourceGrid.headers = {"Метка", "МКОП", "Операнд 1", "Операнд 2"};
    sourceGrid.widths = {24, 22, 27, 27};
    sourceGrid.editable = true;
    sourceGrid.keepEmptyLastRow = true;
    sourceGrid.showRowNumbers = true;
    sourceGrid.name = "Исходный текст";
    sourceGrid.labelColumn = true;
    sourceGrid.create(mainWindow, ID_SOURCE, fonts.mono);

    opsGrid.headers = {"Мнемоника", "Код (hex)", "Длина"};
    opsGrid.widths = {40, 30, 30};
    opsGrid.editable = true;
    opsGrid.keepEmptyLastRow = true;
    opsGrid.showRowNumbers = true;
    opsGrid.name = "Таблица кодов операций";
    opsGrid.create(mainWindow, ID_OPS, fonts.mono);

    // Средняя карточка - первый проход.
    supportGrid.headers = {"Адрес", "Фмт", "КОП | адр.", "Код", "Метка"};
    supportGrid.widths = {12, 10, 19, 32, 27};
    supportGrid.showTooltips = true;
    supportGrid.placeholder = "Появится после первого прохода";
    supportGrid.create(mainWindow, ID_SUPPORT, fonts.mono);

    symbolsGrid.headers = {"Имя", "Адрес", "Внешнее"};
    symbolsGrid.widths = {40, 35, 25};
    symbolsGrid.placeholder = "Появится после первого прохода";
    symbolsGrid.create(mainWindow, ID_SYMBOLS, fonts.mono);

    errors1Grid.headers = {"Где", "Сообщение"};
    errors1Grid.widths = {18, 82};
    errors1Grid.showTooltips = true;
    errors1Grid.create(mainWindow, ID_ERRORS1, fonts.normal);

    // Правая карточка - второй проход.
    headerGrid.headers = {"Имя программы", "Длина", "Адрес загрузки"};
    headerGrid.widths = {38, 22, 40};
    headerGrid.placeholder = "Появится после второго прохода";
    headerGrid.create(mainWindow, ID_HEADER, fonts.mono);

    relocGrid.headers = {"Адрес", "Внешняя ссылка"};
    relocGrid.widths = {40, 60};
    relocGrid.placeholder = "После 2-го прохода";
    relocGrid.create(mainWindow, ID_RELOC, fonts.mono);

    extNamesGrid.headers = {"Адрес", "Имя"};
    extNamesGrid.widths = {45, 55};
    extNamesGrid.placeholder = "После 2-го прохода";
    extNamesGrid.create(mainWindow, ID_EXTNAMES, fonts.mono);

    extRefsGrid.headers = {"Имя"};
    extRefsGrid.widths = {100};
    extRefsGrid.placeholder = "После 2-го прохода";
    extRefsGrid.create(mainWindow, ID_EXTREFS, fonts.mono);

    codeGrid.headers = {"Адрес", "Машинный код", "Команда"};
    codeGrid.widths = {22, 40, 38};
    codeGrid.placeholder = "Появится после второго прохода";
    codeGrid.create(mainWindow, ID_CODE, fonts.mono);

    errors2Grid.headers = {"Где", "Сообщение"};
    errors2Grid.widths = {18, 82};
    errors2Grid.showTooltips = true;
    errors2Grid.create(mainWindow, ID_ERRORS2, fonts.normal);
}

void createButtons() {
    pass1Button = makeButton(mainWindow, ID_PASS1, "Первый проход   F5", BUTTON_PRIMARY);
    pass2Button = makeButton(mainWindow, ID_PASS2, "Второй проход   F6", BUTTON_PRIMARY);
    resetButton = makeButton(mainWindow, ID_RESET, "Очистить результаты", BUTTON_SECONDARY);
    themeButton = makeButton(mainWindow, ID_THEME, darkTheme ? "Светлая тема" : "Тёмная тема", BUTTON_SECONDARY);
    saveButton = makeButton(mainWindow, ID_SAVE, "Сохранить...", BUTTON_SECONDARY);
    EnableWindow(saveButton, FALSE);

    // Выпадающий список примеров.
    examplesList = CreateWindowExW(0, WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST, 0, 0,
                                   10, 10, mainWindow, (HMENU)(INT_PTR)ID_EXAMPLES, GetModuleHandleW(NULL), NULL);
    SendMessageW(examplesList, WM_SETFONT, (WPARAM)fonts.normal, TRUE);
    for (Example example : examples) {
        SendMessageW(examplesList, CB_ADDSTRING, 0, (LPARAM)toWide(example.title).c_str());
    }
    SendMessageW(examplesList, CB_SETCURSEL, 0, 0);
    applyControlTheme(examplesList, L"DarkMode_CFD");

    // Поле "Адрес загрузки". Рамку вокруг него рисуем сами (paint.h).
    addressEdit = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_UPPERCASE | ES_AUTOHSCROLL, 0, 0, 10,
                                  10, mainWindow, (HMENU)(INT_PTR)ID_ADDRESS, GetModuleHandleW(NULL), NULL);
    SendMessageW(addressEdit, WM_SETFONT, (WPARAM)fonts.mono, TRUE);
    SendMessageW(addressEdit, EM_LIMITTEXT, ADDRESS_DIGITS, 0);
}

void createControls(HWND window) {
    mainWindow = window;
    examples = allExamples();
    createGrids();
    createButtons();
    loadExample(0);
}
