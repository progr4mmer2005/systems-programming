// Что делают кнопки и поля окна: проходы, примеры, адрес загрузки, тема.
#pragma once

#include "../asm/first_pass.h"
#include "../asm/second_pass.h"
#include "../asm/object_module.h"
#include "../platform/settings.h"
#include "state.h"

void setStatus(string text, COLORREF color) {
    statusText = text;
    statusColor = color;
    InvalidateRect(mainWindow, &statusBox, FALSE);
}

void redrawWindow() { InvalidateRect(mainWindow, NULL, FALSE); }

// Стереть результаты - при любом изменении входных данных.
void clearResults() {
    pass1 = Pass1Result();
    pass2 = Pass2Result();
    pass1Done = false;
    step1 = STEP_WAITING;
    step2 = STEP_WAITING;
    badSourceRows.clear();
    badOpsRows.clear();
    errors1Grid.placeholder = "Появится после первого прохода";
    errors2Grid.placeholder = "Появится после второго прохода";
    relocGrid.placeholder = "После 2-го прохода";
    extNamesGrid.placeholder = "После 2-го прохода";
    extRefsGrid.placeholder = "После 2-го прохода";
    supportGrid.clear();
    symbolsGrid.clear();
    errors1Grid.clear();
    headerGrid.clear();
    relocGrid.clear();
    extNamesGrid.clear();
    extRefsGrid.clear();
    EnableWindow(saveButton, FALSE);
    codeGrid.clear();
    errors2Grid.clear();
    EnableWindow(pass2Button, FALSE);
    InvalidateRect(sourceGrid.hwnd, NULL, FALSE);
    InvalidateRect(opsGrid.hwnd, NULL, FALSE);
    redrawWindow();
}

// Отметить строки исходного текста и ТКО, в которых нашлись ошибки.
void markBadRows(vector<Error> errors) {
    badSourceRows = vector<bool>(sourceGrid.rowCount(), false);
    badOpsRows = vector<bool>(opsGrid.rowCount(), false);
    for (Error error : errors) {
        if (error.place == "source" && error.row >= 0 && error.row < len(badSourceRows)) badSourceRows[error.row] = true;
        if (error.place == "ops" && error.row >= 0 && error.row < len(badOpsRows)) badOpsRows[error.row] = true;
    }
    InvalidateRect(sourceGrid.hwnd, NULL, FALSE);
    InvalidateRect(opsGrid.hwnd, NULL, FALSE);
}

// Ошибки -> строки таблицы "Где | Сообщение".
Table errorsTable(vector<Error> errors) {
    Table table;
    for (Error error : errors) table.push_back({placeText(error), error.text});
    return table;
}

// Щелчок по ошибке - выделить строку, где она найдена.
void showErrorRow(vector<Error> errors, int index) {
    if (index < 0 || index >= len(errors)) return;
    Error error = errors[index];
    if (error.place == "source") sourceGrid.select(error.row, 1);
    if (error.place == "ops") opsGrid.select(error.row, 0);
}

// ---- адрес загрузки: поле ввода и операнд Start - одно и то же значение ----

// Номер строки со Start (первая непустая строка) или -1.
int startRow() {
    for (int row = 0; row < sourceGrid.rowCount(); row++) {
        if (isEmptyRow(sourceGrid.rows[row])) continue;
        if (sameName(cell(sourceGrid.rows[row], 1), "START")) return row;
        return -1;
    }
    return -1;
}

// Исходный текст изменился -> показать адрес из Start в поле ввода.
void showAddressFromSource() {
    int row = startRow();
    string address = row >= 0 ? sourceGrid.rows[row][2] : "";
    if (windowText(addressEdit) != address) {
        updatingAddress = true;
        setWindowText(addressEdit, address);
        updatingAddress = false;
    }
    EnableWindow(addressEdit, row >= 0);
}

// ---- проходы ----

// Строка вспомогательной таблицы: адрес | МКОП | операнд 1 | операнд 2.
// МКОП команды - её первый байт, уже собранный из битов: код операции * 4 + тип
// адресации (LD с прямой адресацией: 01 * 4 + 1 = 05). У данных - WORD / BYTE.
// Операнды - как в исходном тексте, метки в них заменит адресами второй проход.
Row supportTableRow(Line line) {
    string mkop = line.kind == "command" ? hex(line.firstByte, 2) : line.mnemonic;
    return {hexAddress(line.address), mkop, line.op1, line.op2};
}

void showPass1() {
    sourceGrid.commitEdit();
    opsGrid.commitEdit();
    clearResults();

    pass1 = firstPass(sourceGrid.rows, opsGrid.rows);
    pass1Done = true;

    Table support, symbols;
    for (Line line : pass1.lines) support.push_back(supportTableRow(line));
    for (Symbol symbol : pass1.symbols) symbols.push_back({symbol.name, hexAddress(symbol.address), symbol.external ? "1" : "0"});
    supportGrid.setRows(support);
    symbolsGrid.setRows(symbols);
    errors1Grid.placeholder = "Ошибок нет";
    errors1Grid.setRows(errorsTable(pass1.errors));
    markBadRows(pass1.errors);

    if (pass1.ok) {
        step1 = STEP_OK;
        EnableWindow(pass2Button, TRUE);
        setStatus("Первый проход выполнен: " + countText(len(pass1.lines), "строка", "строки", "строк") + ", " +
                      countText(len(pass1.symbols), "метка", "метки", "меток") + ", длина программы " +
                      hexAddress(pass1.length) + ". Можно запускать второй проход.",
                  colors.ok);
    } else {
        step1 = STEP_FAILED;
        setStatus("Первый проход: " + countText(len(pass1.errors), "ошибка", "ошибки", "ошибок") +
                      ". Щёлкните по ошибке — строка подсветится в таблице.",
                  colors.error);
    }
    redrawWindow();
}

// Двоичный код в виде записей объектного модуля:
//   H - заголовок: адрес загрузки, длина, имя программы;
//   T - строка кода: адрес, длина в байтах, код;
//   E - конец модуля: точка входа (адрес загрузки).
Table codeTable() {
    Table table;
    table.push_back({"H", hexAddress(pass2.start), hexAddress(pass2.length), pass2.programName});
    for (CodeLine line : pass2.code) {
        string size = hex(line.size, line.size > 255 ? 4 : 2);
        table.push_back({"T", hexAddress(line.address), size, line.code});
    }
    table.push_back({"E", hexAddress(pass2.start), "", ""});
    return table;
}

void showPass2() {
    if (!pass1Done || !pass1.ok) return;
    pass2 = secondPass(pass1);

    headerGrid.setRows({{pass2.programName, hexAddress(pass2.length), hexAddress(pass2.start)}});
    codeGrid.setRows(codeTable());
    Table relocations;
    for (Relocation relocation : pass2.relocations) relocations.push_back({hexAddress(relocation.address), relocation.name});
    relocGrid.placeholder = "Пусто";
    relocGrid.setRows(relocations);
    Table names, refs;
    for (Symbol name : pass2.externalNames) names.push_back({hexAddress(name.address), name.name});
    for (Symbol ref : pass2.externalRefs) refs.push_back({ref.name});
    extNamesGrid.placeholder = "Пусто";
    extNamesGrid.setRows(names);
    extRefsGrid.placeholder = "Пусто";
    extRefsGrid.setRows(refs);
    errors2Grid.placeholder = "Ошибок нет";
    errors2Grid.setRows(errorsTable(pass2.errors));
    markBadRows(pass2.errors);
    EnableWindow(pass2Button, FALSE);  // по методичке после второго прохода кнопка снова неактивна

    if (pass2.ok) {
        step2 = STEP_OK;
        EnableWindow(saveButton, TRUE);
        setStatus("Второй проход выполнен: объектный модуль готов, его можно сохранить в файл.", colors.ok);
    } else {
        step2 = STEP_FAILED;
        setStatus("Второй проход: " + countText(len(pass2.errors), "ошибка", "ошибки", "ошибок") +
                      ". Исправьте исходный текст и повторите.",
                  colors.error);
    }
    redrawWindow();
}

// Кнопки "Первый проход" и "Второй проход".
void runPass1() {
    lastPass = 1;
    showPass1();
}

void runPass2() {
    if (!pass1Done || !pass1.ok) return;
    lastPass = 2;
    showPass2();
}

// Данные изменились: заново выполнить проходы, до которых дошёл пользователь.
// Если первый проход теперь с ошибками, второй не выполняется, но после
// исправления ошибок выполнится снова.
void recalculate() {
    if (lastPass == 0) {
        clearResults();
        setStatus("Данные изменены. Нажмите «Первый проход».", colors.muted);
        return;
    }
    showPass1();
    if (lastPass == 2) showPass2();
}

// Кнопка "Очистить" и выбор другого примера.
void resetResults() {
    lastPass = 0;
    clearResults();
}

// Пользователь что-то поменял в исходном тексте или ТКО.
void onInputChanged() {
    showAddressFromSource();
    recalculate();
}

// Пользователь изменил поле адреса загрузки -> записать адрес в операнд Start.
void onAddressEdited() {
    int row = startRow();
    if (updatingAddress || row < 0) return;
    sourceGrid.rows[row][2] = trim(windowText(addressEdit));
    sourceGrid.refresh();
    recalculate();
}

void loadExample(int index) {
    if (index < 0 || index >= len(examples)) return;
    Example example = examples[index];
    sourceGrid.setRows(example.source);
    opsGrid.setRows(example.ops);
    sourceGrid.select(0, 1);
    sourceGrid.scrollToTop();
    opsGrid.scrollToTop();
    resetResults();
    showAddressFromSource();
    setStatus("Загружен пример «" + example.title + "». Нажмите «Первый проход» (F5).", colors.muted);
}

// Кнопка "Сохранить...": записать объектный модуль в текстовый файл.
void saveObjectModule() {
    if (!pass2.ok || !dialogs.GetSaveFileNameW) return;
    wstring defaultName = toWide(pass2.programName + ".txt");
    vector<wchar_t> path(MAX_PATH, 0);
    for (int i = 0; i < (int)defaultName.size() && i < MAX_PATH - 1; i++) path[i] = defaultName[i];

    OPENFILENAMEW dialog = {};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = mainWindow;
    dialog.lpstrFilter = L"Текстовые файлы (*.txt)\0*.txt\0Все файлы\0*.*\0";
    dialog.lpstrFile = path.data();
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrDefExt = L"txt";
    dialog.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    if (!dialogs.GetSaveFileNameW(&dialog)) return;  // нажали "Отмена"

    FILE* file = _wfopen(path.data(), L"wb");
    if (file == NULL) {
        setStatus("Не удалось сохранить файл " + fromWide(path.data()), colors.error);
        return;
    }
    string text = withWindowsNewlines(objectModuleText(pass2));
    fwrite(text.data(), 1, text.size(), file);
    fclose(file);
    setStatus("Объектный модуль сохранён в файл " + fromWide(path.data()), colors.ok);
}

void toggleTheme() {
    setDarkTheme(!darkTheme);
    saveDarkTheme(darkTheme);
    for (Grid* grid : allGrids) grid->applyTheme();
    applyControlTheme(examplesList, L"DarkMode_CFD");
    applyTitleBarTheme(mainWindow);
    setWindowText(themeButton, darkTheme ? "Светлая тема" : "Тёмная тема");
    RedrawWindow(mainWindow, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
}
