#pragma once

#include "../asm/one_pass.h"
#include "state.h"

int sourceRowOf(int gridId, int row);

void setStatus(string text) { SendMessageW(statusBar, SB_SETTEXTW, 0, (LPARAM)toWide(text).c_str()); }

void redrawWindow() { InvalidateRect(mainWindow, NULL, FALSE); }

const string PRESS_STEP = "Нажмите «Шаг» (F10) или «Полный проход» (F5).";

int firstSourceRow() {
    for (int row = 0; row < sourceGrid.rowCount(); row++) {
        if (!isEmptyRow(sourceGrid.rows[row])) return row;
    }
    return -1;
}

void showNextRow(int row) {
    nextSourceRow = row;
    InvalidateRect(sourceGrid.hwnd, NULL, FALSE);
    sourceGrid.showRow(row);
}

void clearResults() {
    assembly = Assembly();
    badSourceRows.clear();
    badOpsRows.clear();
    symbolsGrid.placeholder = "Появится после первого шага";
    errorsGrid.placeholder = "Появится после первого шага";
    codeGrid.placeholder = "Появится после первого шага";
    headerGrid.placeholder = "Заполнится на директиве End";
    relocGrid.placeholder = "Появится после первого шага";
    extNamesGrid.placeholder = "Появится после первого шага";
    extRefsGrid.placeholder = "Появится после первого шага";
    symbolsGrid.clear();
    relocGrid.clear();
    extNamesGrid.clear();
    extRefsGrid.clear();
    moduleRecords.clear();
    EnableWindow(saveButton, FALSE);
    errorsGrid.clear();
    headerGrid.clear();
    codeGrid.clear();
    EnableWindow(stepButton, TRUE);
    EnableWindow(runButton, TRUE);
    InvalidateRect(opsGrid.hwnd, NULL, FALSE);
    showNextRow(firstSourceRow());
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

Table symbolsTable() {
    Table table;
    for (Symbol symbol : visibleSymbols(assembly)) {
        bool external = false;
        for (ModuleSection section : assembly.sections)
            if (sameName(section.name, symbol.section) && findSymbol(section.extDefs, symbol.name) != -1) external = true;
        if (sameName(assembly.programName, symbol.section) && findSymbol(assembly.extDefs, symbol.name) != -1) external = true;
        table.push_back({symbol.section, symbol.name, symbol.defined ? hexAddress(symbol.address) : "", external ? "1" : "0", addressList(symbol.waiting)});
    }
    return table;
}

Table codeTable() {
    Table table;
    for (ModuleRecord record : moduleRecords) table.push_back(record.cells);
    return table;
}

string stepText() {
    string text = "Шаг " + to_string(assembly.steps);
    if (assembly.lastRow < 0) return text;
    text += ", строка " + to_string(assembly.lastRow + 1) + ": " + assembly.done;
    for (Error error : assembly.errors) {
        if (error.place != "source" || error.row != assembly.lastRow) continue;
        text += (assembly.done == "" ? "" : "; ") + string("ошибка - ") + error.text;
        break;
    }
    return text;
}

void showAssembly() {
    moduleRecords = objectModule(assembly);
    symbolsGrid.setRows(symbolsTable());
    codeGrid.setRows(codeTable());
    Table relocations, names, refs;
    for (Relocation relocation : assembly.relocations) relocations.push_back({hexAddress(relocation.address), relocation.name});
    for (Symbol name : externalNames(assembly)) names.push_back({name.defined ? hexAddress(name.address) : "", name.name});
    for (Symbol ref : assembly.extRefs) refs.push_back({ref.name});
    relocGrid.placeholder = "Пусто: настраивать пока нечего";
    relocGrid.setRows(relocations);
    extNamesGrid.placeholder = "Пусто";
    extNamesGrid.setRows(names);
    extRefsGrid.placeholder = "Пусто";
    extRefsGrid.setRows(refs);
    errorsGrid.placeholder = "Ошибок нет";
    errorsGrid.setRows(errorsTable(assembly.errors));
    markBadRows(assembly.errors);
    Table headers;
    for (ModuleSection section : assembly.sections) headers.push_back({section.name, hexAddress(section.length), "000000"});
    if (!assembly.ended && assembly.startRow >= 0) headers.push_back({assembly.programName, hexAddress(assembly.counter - assembly.start), "000000"});
    headerGrid.setRows(headers);

    EnableWindow(stepButton, !assembly.finished);
    EnableWindow(runButton, !assembly.finished);
    EnableWindow(saveButton, assembly.finished && assembly.ok);
    if (assembly.lastRow >= 0) {
        sourceGrid.select(assembly.lastRow, sourceGrid.currentCol);
        for (Grid* grid : {&symbolsGrid, &headerGrid, &relocGrid, &extNamesGrid, &extRefsGrid, &codeGrid}) {
            grid->unselect();
            for (int row = 0; row < grid->rowCount(); row++) {
                if (sourceRowOf(grid->id, row) != assembly.lastRow) continue;
                grid->select(row, 0);
                break;
            }
        }
    }
    showNextRow(assembly.finished ? -1 : assembly.nextRow);

    int errors = len(assembly.errors);
    if (!assembly.finished) {
        setStatus(stepText() + ". Следующая - строка " + to_string(assembly.nextRow + 1) + ".");
    } else if (errors == 0) {
        setStatus(stepText() + ". Готово: объектный модуль сформирован, его можно сохранить в файл.");
    } else {
        string text = assembly.steps > 0 ? stepText() + ". " : "";
        setStatus(text + "Ассемблирование закончено: " + countText(errors, "ошибка", "ошибки", "ошибок") +
                  ". Щёлкните по ошибке - строка подсветится в таблице.");
    }
    redrawWindow();
}

int wantedSteps = 0;
bool wantAll = false;

void beginIfNeeded() {
    if (assembling) return;
    sourceGrid.commitEdit();
    opsGrid.commitEdit();
    clearResults();
    assembly = beginAssembly(sourceGrid.rows, opsGrid.rows, addressingMode);
    assembling = true;
}

void stepOnce() {
    sourceGrid.commitEdit();
    opsGrid.commitEdit();
    beginIfNeeded();
    stepAssembly(assembly);
    wantedSteps = assembly.steps;
    showAssembly();
}

void runToEnd() {
    sourceGrid.commitEdit();
    opsGrid.commitEdit();
    beginIfNeeded();
    runAssembly(assembly);
    wantAll = true;
    showAssembly();
}

void resetAssembly() {
    assembling = false;
    wantedSteps = 0;
    wantAll = false;
    clearResults();
}

void recalculate() {
    if (!assembling) {
        clearResults();
        setStatus("Данные изменены. " + PRESS_STEP);
        return;
    }
    assembly = beginAssembly(sourceGrid.rows, opsGrid.rows, addressingMode);
    if (wantAll)
        runAssembly(assembly);
    else
        while (assembly.steps < wantedSteps && !assembly.finished) stepAssembly(assembly);
    showAssembly();
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
    resetAssembly();
    setStatus("Загружен пример «" + example.title + "». " + PRESS_STEP);
}

void saveObjectModule() {
    if (!assembly.finished || !assembly.ok || !dialogs.GetSaveFileNameW) return;
    wstring defaultName = toWide(assembly.programName + ".txt");
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
