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

    symbolsGrid.headers = {"Метка", "Адрес", "Команды, ждущие адрес"};
    symbolsGrid.widths = {30, 24, 46};
    symbolsGrid.showTooltips = true;
    symbolsGrid.fitText = true;
    symbolsGrid.placeholder = "Появится после первого шага";
    symbolsGrid.create(mainWindow, ID_SYMBOLS, fonts.normal);

    errorsGrid.headers = {"Где", "Сообщение"};
    errorsGrid.widths = {18, 82};
    errorsGrid.showTooltips = true;
    errorsGrid.fitText = true;
    errorsGrid.create(mainWindow, ID_ERRORS, fonts.normal);

    headerGrid.headers = {"Имя программы", "Длина", "Адрес загрузки"};
    headerGrid.widths = {34, 33, 33};
    headerGrid.placeholder = "Заполнится на директиве End";
    headerGrid.create(mainWindow, ID_HEADER, fonts.normal);

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

    examplesList = CreateWindowExW(0, WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST, 0, 0,
                                   10, 10, mainWindow, (HMENU)(INT_PTR)ID_EXAMPLES, GetModuleHandleW(NULL), NULL);
    SendMessageW(examplesList, WM_SETFONT, (WPARAM)fonts.normal, TRUE);
    for (Example example : examples) {
        SendMessageW(examplesList, CB_ADDSTRING, 0, (LPARAM)toWide(example.title).c_str());
    }
    SendMessageW(examplesList, CB_SETCURSEL, 0, 0);

    addressEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_UPPERCASE | ES_AUTOHSCROLL, 0, 0, 10,
                                  10, mainWindow, (HMENU)(INT_PTR)ID_ADDRESS, GetModuleHandleW(NULL), NULL);
    SendMessageW(addressEdit, WM_SETFONT, (WPARAM)fonts.normal, TRUE);
    SendMessageW(addressEdit, EM_LIMITTEXT, 8, 0);
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
