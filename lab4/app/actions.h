#pragma once

#include "../asm/one_pass.h"
#include "state.h"

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
    symbolsGrid.clear();
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

int startRow() {
    int row = firstSourceRow();
    if (row >= 0 && sameName(cell(sourceGrid.rows[row], 1), "START")) return row;
    return -1;
}

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

int codeOffset() { return assembly.startRow >= 0 ? 1 : 0; }

Table symbolsTable() {
    Table table;
    for (Symbol symbol : assembly.symbols)
        table.push_back({symbol.name, symbol.defined ? hexAddress(symbol.address) : "", addressList(symbol.waiting)});
    return table;
}

Table codeTable() {
    Table table;
    if (assembly.startRow >= 0)
        table.push_back({"H", assembly.programName, hexAddress(assembly.start), assembly.ended ? hexAddress(assembly.length) : "", ""});
    for (CodeLine line : assembly.code) table.push_back({"T", "", hexAddress(line.address), hexLength(line.size), line.code});
    if (assembly.ended) table.push_back({"E", "", hexAddress(assembly.entry), "", ""});
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
    symbolsGrid.setRows(symbolsTable());
    codeGrid.setRows(codeTable());
    errorsGrid.placeholder = "Ошибок нет";
    errorsGrid.setRows(errorsTable(assembly.errors));
    markBadRows(assembly.errors);
    if (assembly.ended)
        headerGrid.setRows({{assembly.programName, hexAddress(assembly.length), hexAddress(assembly.start)}});
    else
        headerGrid.clear();

    EnableWindow(stepButton, !assembly.finished);
    EnableWindow(runButton, !assembly.finished);
    if (assembly.lastRow >= 0) {
        sourceGrid.select(assembly.lastRow, sourceGrid.currentCol);
        for (Grid* grid : {&symbolsGrid, &headerGrid, &codeGrid}) grid->unselect();
        int line = -1;
        for (int i = 0; i < len(assembly.code); i++) {
            if (assembly.code[i].row == assembly.lastRow) line = i;
        }
        if (line >= 0) codeGrid.select(line + codeOffset(), 0);
        if (assembly.lastRow == assembly.endRow) codeGrid.select(codeGrid.rowCount() - 1, 0);
        for (int i = 0; i < len(assembly.symbols); i++) {
            if (assembly.symbols[i].row == assembly.lastRow) symbolsGrid.select(i, 0);
        }
    }
    showNextRow(assembly.finished ? -1 : assembly.nextRow);

    int errors = len(assembly.errors);
    if (!assembly.finished) {
        setStatus(stepText() + ". Следующая - строка " + to_string(assembly.nextRow + 1) + ".");
    } else if (errors == 0) {
        setStatus(stepText() + ". Готово: объектный модуль сформирован.");
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
    assembly = beginAssembly(sourceGrid.rows, opsGrid.rows);
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
    assembly = beginAssembly(sourceGrid.rows, opsGrid.rows);
    if (wantAll)
        runAssembly(assembly);
    else
        while (assembly.steps < wantedSteps && !assembly.finished) stepAssembly(assembly);
    showAssembly();
}

void onInputChanged() {
    showAddressFromSource();
    recalculate();
}

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
    resetAssembly();
    showAddressFromSource();
    setStatus("Загружен пример «" + example.title + "». " + PRESS_STEP);
}
