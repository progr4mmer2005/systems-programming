// Grid: редактирование всей таблицы как обычного текста.
//
// Одна строка текста - одна строка таблицы. Столбцы разделяются пробелами или Tab,
// текст в кавычках ("Hello world") не разбивается. Если у таблицы первый столбец -
// метка (labelColumn), строка, которая начинается с пробела или Tab, - без метки:
//
//   Exampl  Start  1000
//           LD     R1  One
#pragma once

#include "grid.h"
#include "text_editor.h"

// Сколько символов в строке UTF-8 (для выравнивания столбцов).
int charCount(string s) {
    int count = 0;
    for (char c : s) {
        if ((c & 0xC0) != 0x80) count++;
    }
    return count;
}

bool isSpace(char c) { return c == ' ' || c == '\t'; }

// Слова строки через пробелы и Tab; текст в кавычках - одно слово.
vector<string> splitWords(string line) {
    vector<string> words;
    string word;
    bool inQuotes = false;
    for (char c : line) {
        if (c == '"') inQuotes = !inQuotes;
        if (isSpace(c) && !inQuotes) {
            if (word != "") words.push_back(word);
            word = "";
        } else {
            word += c;
        }
    }
    if (word != "") words.push_back(word);
    return words;
}

string Grid::toText() {
    int columns = columnCount();

    // Ширина каждого столбца, кроме последнего, - по самому длинному значению.
    vector<int> width(columns, 0);
    for (Row row : rows) {
        for (int col = 0; col < columns - 1; col++) width[col] = max(width[col], charCount(cell(row, col)));
    }
    if (labelColumn) width[0] = max(width[0], 6);  // чтобы строка без метки начиналась с отступа

    // Пустые строки в конце не нужны.
    int count = rowCount();
    while (count > 0 && isEmptyRow(rows[count - 1])) count--;

    string text;
    for (int r = 0; r < count; r++) {
        string line;
        for (int col = 0; col < columns; col++) {
            string value = cell(rows[r], col);
            line += value;
            if (col < columns - 1) line += string(width[col] - charCount(value) + 2, ' ');
        }
        text += trimRight(line) + "\n";
    }
    return text;
}

Table Grid::fromText(string text) {
    Table table;
    string line;
    text += "\n";
    for (char c : text) {
        if (c != '\n') {
            line += c;
            continue;
        }
        vector<string> words = splitWords(line);
        Row row;
        if (labelColumn && len(words) > 0 && isSpace(line[0])) row.push_back("");
        for (string word : words) {
            if (len(row) < columnCount())
                row.push_back(word);
            else
                row.back() += " " + word;  // лишние слова - в последний столбец
        }
        row.resize(columnCount());
        table.push_back(row);
        line = "";
    }
    while (len(table) > 0 && isEmptyRow(table.back())) table.pop_back();
    return table;
}

void Grid::editAsText() {
    commitEdit();
    string hint = "Одна строка - одна строка таблицы. Столбцы разделяются пробелами или Tab.";
    if (labelColumn) hint += " Строка, которая начинается с пробела, - без метки.";
    hint += " Ctrl+Enter - применить, Esc - отмена.";

    TextEditorResult result = openTextEditor(GetAncestor(hwnd, GA_ROOT), name + " - редактирование как текст", hint, toText());
    SetFocus(hwnd);
    if (!result.ok) return;

    Table newRows = fromText(result.text);
    if (newRows == fromText(toText())) return;  // ничего не поменяли
    rows = newRows;
    changed();
    select(min(max(currentRow, 0), rowCount() - 1), currentCol);
}
