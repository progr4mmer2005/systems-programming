#pragma once

#include "../asm/first_pass.h"
#include "../asm/second_pass.h"
#include "state.h"

void setStatus(string text) { SendMessageW(statusBar, SB_SETTEXTW, 0, (LPARAM)toWide(text).c_str()); }

void redrawWindow() { InvalidateRect(mainWindow, NULL, FALSE); }

void clearResults() {
    pass1 = Pass1Result();
    pass2 = Pass2Result();
    pass1Done = false;
    badSourceRows.clear();
    badOpsRows.clear();
    errors1Grid.placeholder = "Появится после первого прохода";
    errors2Grid.placeholder = "Появится после второго прохода";
    headerGrid.placeholder = "Появится после второго прохода";
    relocGrid.placeholder = "Появится после второго прохода";
    extNamesGrid.placeholder = "После 2-го прохода";
    extRefsGrid.placeholder = "После 2-го прохода";
    supportGrid.clear();
    symbolsGrid.clear();
    errors1Grid.clear();
    relocGrid.clear();
    extNamesGrid.clear();
    extRefsGrid.clear();
    moduleRecords.clear();
    EnableWindow(saveButton, FALSE);
    headerGrid.clear();
    codeGrid.clear();
    errors2Grid.clear();
    EnableWindow(pass2Button, FALSE);
    InvalidateRect(sourceGrid.hwnd, NULL, FALSE);
    InvalidateRect(opsGrid.hwnd, NULL, FALSE);
    redrawWindow();
}

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

Table errorsTable(vector<Error> errors) {
    Table table;
    for (Error error : errors) table.push_back({placeText(error), error.text});
    return table;
}

void showErrorRow(vector<Error> errors, int index) {
    if (index < 0 || index >= len(errors)) return;
    Error error = errors[index];
    if (error.place == "source") sourceGrid.select(error.row, 1);
    if (error.place == "ops") opsGrid.select(error.row, 0);
}

Row supportTableRow(Line line) {
    if (line.kind == "START") return {pass1.programName, "START", hexAddress(pass1.start), ""};
    string mkop = line.kind == "command" ? hex(line.firstByte, 2) : line.mnemonic;
    return {hexAddress(line.address), mkop, line.op1, line.op2};
}

void showPass1() {
    sourceGrid.commitEdit();
    opsGrid.commitEdit();
    clearResults();

    pass1 = firstPass(sourceGrid.rows, opsGrid.rows, addressingMode);
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
        EnableWindow(pass2Button, TRUE);
        int commands = len(pass1.lines) - 1;
        setStatus("Первый проход выполнен: " + countText(commands, "строка", "строки", "строк") + ", " +
                      countText(len(pass1.symbols), "метка", "метки", "меток") + ", длина программы " +
                      hexAddress(pass1.length) + ". Можно запускать второй проход.");
    } else {
        setStatus("Первый проход: " + countText(len(pass1.errors), "ошибка", "ошибки", "ошибок") +
                      ". Щёлкните по ошибке — строка подсветится в таблице.");
    }
    redrawWindow();
}

Table codeTable() {
    Table table;
    for (ModuleRecord record : moduleRecords) table.push_back(record.cells);
    return table;
}

void showPass2() {
    if (!pass1Done || !pass1.ok) return;
    pass2 = secondPass(pass1);

    moduleRecords = objectModule(pass1, pass2);
    headerGrid.setRows({{pass2.programName, hexAddress(pass2.length), hexAddress(pass2.start)}});
    codeGrid.setRows(codeTable());
    Table relocations, names, refs;
    for (Relocation relocation : pass2.relocations) relocations.push_back({hexAddress(relocation.address), relocation.name});
    for (Symbol name : pass2.externalNames) names.push_back({hexAddress(name.address), name.name});
    for (Symbol ref : pass2.externalRefs) refs.push_back({ref.name});
    relocGrid.placeholder = "Пусто: настраивать нечего";
    relocGrid.setRows(relocations);
    extNamesGrid.placeholder = "Пусто";
    extNamesGrid.setRows(names);
    extRefsGrid.placeholder = "Пусто";
    extRefsGrid.setRows(refs);
    errors2Grid.placeholder = "Ошибок нет";
    errors2Grid.setRows(errorsTable(pass2.errors));
    markBadRows(pass2.errors);
    EnableWindow(pass2Button, FALSE);

    if (pass2.ok) {
        EnableWindow(saveButton, TRUE);
        setStatus("Второй проход выполнен: объектный модуль готов, в таблице настройки " +
                      countText(len(pass2.relocations), "строка", "строки", "строк") + ". Его можно сохранить в файл.");
    } else {
        setStatus("Второй проход: " + countText(len(pass2.errors), "ошибка", "ошибки", "ошибок") +
                      ". Исправьте исходный текст и повторите.");
    }
    redrawWindow();
}

void runPass1() {
    lastPass = 1;
    showPass1();
}

void runPass2() {
    if (!pass1Done || !pass1.ok) return;
    lastPass = 2;
    showPass2();
}

void recalculate() {
    if (lastPass == 0) {
        clearResults();
        setStatus("Данные изменены. Нажмите «Первый проход».");
        return;
    }
    showPass1();
    if (lastPass == 2) showPass2();
}

void resetResults() {
    lastPass = 0;
    clearResults();
}

void onInputChanged() { recalculate(); }

void onModeChanged(int mode) {
    if (mode < MODE_BOTH || mode > MODE_RELATIVE) return;
    addressingMode = mode;
    recalculate();
}

void loadExample(int index) {
    if (index < 0 || index >= len(examples)) return;
    Example example = examples[index];
    sourceGrid.setRows(example.source);
    opsGrid.setRows(example.ops);
    addressingMode = example.mode;
    SendMessageW(modeList, CB_SETCURSEL, addressingMode, 0);
    sourceGrid.select(0, 1);
    sourceGrid.scrollToTop();
    opsGrid.scrollToTop();
    resetResults();
    setStatus("Загружен пример «" + example.title + "». Нажмите «Первый проход» (F5).");
}

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
    if (!dialogs.GetSaveFileNameW(&dialog)) return;

    FILE* file = _wfopen(path.data(), L"wb");
    if (file == NULL) {
        setStatus("Не удалось сохранить файл " + fromWide(path.data()));
        return;
    }
    string text = withWindowsNewlines(objectModuleText(moduleRecords));
    fwrite(text.data(), 1, text.size(), file);
    fclose(file);
    setStatus("Объектный модуль сохранён в файл " + fromWide(path.data()));
}

