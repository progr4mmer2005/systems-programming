#pragma once

#include "../ui/button.h"
#include "actions.h"
#include "state.h"

void createGrids() {
    sourceGrid.headers = {"Метка", "МКОП", "Операнд 1", "Операнд 2"};
    sourceGrid.widths = {24, 22, 27, 27};
    sourceGrid.editable = true;
    sourceGrid.keepEmptyLastRow = true;
    sourceGrid.showRowNumbers = true;
    sourceGrid.name = "Исходный текст";
    sourceGrid.labelColumn = true;
    sourceGrid.create(mainWindow, ID_SOURCE, fonts.normal);

    opsGrid.headers = {"Мнемоника", "Код (hex)", "Длина"};
    opsGrid.widths = {40, 30, 30};
    opsGrid.editable = true;
    opsGrid.keepEmptyLastRow = true;
    opsGrid.showRowNumbers = true;
    opsGrid.name = "Таблица кодов операций";
    opsGrid.create(mainWindow, ID_OPS, fonts.normal);

    supportGrid.headers = {"Адрес", "МКОП", "Операнд 1", "Операнд 2"};
    supportGrid.widths = {20, 18, 31, 31};
    supportGrid.showTooltips = true;
    supportGrid.placeholder = "Появится после первого прохода";
    supportGrid.create(mainWindow, ID_SUPPORT, fonts.normal);

    symbolsGrid.headers = {"Метка", "Адрес"};
    symbolsGrid.widths = {50, 50};
    symbolsGrid.placeholder = "Появится после первого прохода";
    symbolsGrid.create(mainWindow, ID_SYMBOLS, fonts.normal);

    errors1Grid.headers = {"Где", "Сообщение"};
    errors1Grid.widths = {18, 82};
    errors1Grid.showTooltips = true;
    errors1Grid.fitText = true;
    errors1Grid.create(mainWindow, ID_ERRORS1, fonts.normal);

    headerGrid.headers = {"Имя программы", "Длина", "Начальный адрес"};
    headerGrid.widths = {34, 33, 33};
    headerGrid.placeholder = "Появится после второго прохода";
    headerGrid.create(mainWindow, ID_HEADER, fonts.normal);
    relocGrid.headers = {"Адрес команды"};
    relocGrid.widths = {100};
    relocGrid.placeholder = "Появится после второго прохода";
    relocGrid.create(mainWindow, ID_RELOC, fonts.normal);

    codeGrid.headers = {"Тип", "Имя", "Адрес", "Длина", "Машинный код"};
    codeGrid.widths = {9, 17, 20, 14, 40};
    codeGrid.showTooltips = true;
    codeGrid.placeholder = "Появится после второго прохода";
    codeGrid.create(mainWindow, ID_CODE, fonts.normal);

    errors2Grid.headers = {"Где", "Сообщение"};
    errors2Grid.widths = {18, 82};
    errors2Grid.showTooltips = true;
    errors2Grid.fitText = true;
    errors2Grid.create(mainWindow, ID_ERRORS2, fonts.normal);
}

void createButtons() {
    pass1Button = makeButton(mainWindow, ID_PASS1, "Первый проход");
    pass2Button = makeButton(mainWindow, ID_PASS2, "Второй проход");
    resetButton = makeButton(mainWindow, ID_RESET, "Очистить результаты");

    examplesList = CreateWindowExW(0, WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST, 0, 0,
                                   10, 10, mainWindow, (HMENU)(INT_PTR)ID_EXAMPLES, GetModuleHandleW(NULL), NULL);
    SendMessageW(examplesList, WM_SETFONT, (WPARAM)fonts.normal, TRUE);
    for (Example example : examples) {
        SendMessageW(examplesList, CB_ADDSTRING, 0, (LPARAM)toWide(example.title).c_str());
    }
    SendMessageW(examplesList, CB_SETCURSEL, 0, 0);

    modeList = CreateWindowExW(0, WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST, 0, 0, 10,
                               10, mainWindow, (HMENU)(INT_PTR)ID_MODE, GetModuleHandleW(NULL), NULL);
    SendMessageW(modeList, WM_SETFONT, (WPARAM)fonts.normal, TRUE);
    for (int mode : {MODE_BOTH, MODE_DIRECT, MODE_RELATIVE}) {
        SendMessageW(modeList, CB_ADDSTRING, 0, (LPARAM)toWide(modeName(mode)).c_str());
    }
    SendMessageW(modeList, CB_SETCURSEL, addressingMode, 0);
}

void createStatusBar() {
    statusBar = CreateWindowExW(0, STATUSCLASSNAMEW, L"", WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, mainWindow,
                                (HMENU)(INT_PTR)ID_STATUS, GetModuleHandleW(NULL), NULL);
    SendMessageW(statusBar, WM_SETFONT, (WPARAM)fonts.normal, TRUE);
}

void createControls(HWND window) {
    mainWindow = window;
    examples = allExamples();
    createGrids();
    createButtons();
    createStatusBar();
    loadExample(0);
}
