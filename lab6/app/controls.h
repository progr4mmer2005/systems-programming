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

    symbolsGrid.headers = {"Секция", "Метка", "Адрес", "Внешнее", "Команды, ждущие адрес"};
    symbolsGrid.widths = {18, 20, 18, 14, 30};
    symbolsGrid.showTooltips = true;
    symbolsGrid.fitText = true;
    symbolsGrid.placeholder = "Появится после первого шага";
    symbolsGrid.create(mainWindow, ID_SYMBOLS, fonts.normal);

    errorsGrid.headers = {"Где", "Сообщение"};
    errorsGrid.widths = {18, 82};
    errorsGrid.showTooltips = true;
    errorsGrid.fitText = true;
    errorsGrid.create(mainWindow, ID_ERRORS, fonts.normal);

    headerGrid.headers = {"Имя", "Длина", "Начальный адрес"};
    headerGrid.widths = {30, 28, 42};
    headerGrid.placeholder = "Заполнится на директиве End";
    headerGrid.create(mainWindow, ID_HEADER, fonts.normal);

    relocGrid.headers = {"Адрес команды", "Внешняя ссылка"};
    relocGrid.widths = {50, 50};
    relocGrid.placeholder = "Появится после первого шага";
    relocGrid.create(mainWindow, ID_RELOC, fonts.normal);

    extNamesGrid.headers = {"Адрес", "Имя"};
    extNamesGrid.widths = {50, 50};
    extNamesGrid.placeholder = "Появится после первого шага";
    extNamesGrid.create(mainWindow, ID_EXTNAMES, fonts.normal);

    extRefsGrid.headers = {"Имя"};
    extRefsGrid.widths = {100};
    extRefsGrid.placeholder = "Появится после первого шага";
    extRefsGrid.create(mainWindow, ID_EXTREFS, fonts.normal);

    codeGrid.headers = {"Тип", "Имя", "Адрес", "Длина", "Машинный код"};
    codeGrid.widths = {9, 17, 20, 14, 40};
    codeGrid.showTooltips = true;
    codeGrid.placeholder = "Появится после первого шага";
    codeGrid.create(mainWindow, ID_CODE, fonts.normal);
}

void createButtons() {
    stepButton = makeButton(mainWindow, ID_STEP, "Шаг");
    runButton = makeButton(mainWindow, ID_RUN, "Полный проход");
    resetButton = makeButton(mainWindow, ID_RESET, "Сброс");
    saveButton = makeButton(mainWindow, ID_SAVE, "Сохранить...");
    EnableWindow(saveButton, FALSE);

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
