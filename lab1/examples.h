#pragma once

#include <string>
#include <vector>

using namespace std;

#include "util/table.h"

struct Example {
    string title;
    Table source;
    Table ops;
    int failsAt;
};

Table defaultOps() {
    return {
        {"JMP", "01", "4"},
        {"LOADR1", "02", "4"},
        {"LOADR2", "03", "4"},
        {"ADD", "04", "2"},
        {"SAVER1", "05", "4"},
        {"INT", "06", "2"},
        {"SUB", "07", "2"},
        {"NOP", "08", "1"},
        {"CALL", "09", "3"},
    };
}

Table referenceSource() {
    return {
        {"PROG", "START", "100", ""},
        {"", "JMP", "L1", ""},
        {"A1", "RESB", "10", ""},
        {"A2", "RESW", "20", ""},
        {"B1", "WORD", "40", ""},
        {"B2", "BYTE", "X\"2F4C008A\"", ""},
        {"B3", "BYTE", "C\"Hello!\"", ""},
        {"B4", "BYTE", "12", ""},
        {"L1", "LOADR1", "B1", ""},
        {"", "LOADR2", "B4", ""},
        {"", "ADD", "R1", "R2"},
        {"", "SAVER1", "B1", ""},
        {"", "INT", "200", ""},
        {"", "END", "100", ""},
    };
}

Table allFormatsSource() {
    return {
        {"PROG", "START", "1000", ""},
        {"", "LOADR1", "One", ""},
        {"", "LOADR2", "Two", ""},
        {"", "ADD", "R1", "R2"},
        {"", "SAVER1", "Rez", ""},
        {"", "CALL", "Show", ""},
        {"", "NOP", "", ""},
        {"Show", "INT", "5", ""},
        {"", "JMP", "Exit", ""},
        {"One", "WORD", "1", ""},
        {"Two", "WORD", "2", ""},
        {"Rez", "WORD", "?", ""},
        {"Text", "BYTE", "\"Hello\"", ""},
        {"Buf", "RESB", "4", ""},
        {"Exit", "NOP", "", ""},
        {"", "END", "", ""},
    };
}

Table withRow(Table table, int index, Row row) {
    table[index] = row;
    return table;
}

vector<Example> allExamples() {
    Table good = referenceSource();

    Table noEnd = good;
    noEnd.pop_back();

    Table badOps = defaultOps();
    badOps[1] = {"LOADR1", "01", "4"};

    Table manyErrors = withRow(good, 9, {"", "LOADR3", "B4", ""});
    manyErrors = withRow(manyErrors, 7, {"B1", "BYTE", "12", ""});
    manyErrors = withRow(manyErrors, 5, {"B2", "BYTE", "X\"2F4C008\"", ""});

    Table manyUndefined = withRow(good, 1, {"", "JMP", "L2", ""});
    manyUndefined = withRow(manyUndefined, 11, {"", "SAVER1", "B5", ""});

    return {
        {"Без ошибок", good, defaultOps(), 0},
        {"Без ошибок: все 4 формата команд", allFormatsSource(), defaultOps(), 0},
        {"Без ошибок: точка входа - метка (END L1)", withRow(good, 13, {"", "END", "L1", ""}), defaultOps(), 0},
        {"Ошибка: метка определена дважды", withRow(good, 7, {"B1", "BYTE", "12", ""}), defaultOps(), 1},
        {"Ошибка: метка - зарезервированное слово", withRow(good, 4, {"ADD", "WORD", "40", ""}), defaultOps(), 1},
        {"Ошибка: неизвестная команда", withRow(good, 9, {"", "LOADR3", "B4", ""}), defaultOps(), 1},
        {"Ошибка: операнды не подходят формату", withRow(good, 10, {"", "ADD", "R1", "B1"}), defaultOps(), 1},
        {"Ошибка: неверный адрес загрузки", withRow(good, 0, {"PROG", "START", "10G", ""}), defaultOps(), 1},
        {"Ошибка: нет директивы END", noEnd, defaultOps(), 1},
        {"Ошибка: неверная точка входа", withRow(good, 13, {"", "END", "50", ""}), defaultOps(), 1},
        {"Ошибка: метки точки входа нет (END L9)", withRow(good, 13, {"", "END", "L9", ""}), defaultOps(), 1},
        {"Ошибка: метка на строке END", withRow(good, 13, {"FIN", "END", "100", ""}), defaultOps(), 1},
        {"Ошибка: относительная адресация [L1]", withRow(good, 1, {"", "JMP", "[L1]", ""}), defaultOps(), 1},
        {"Ошибка: BYTE больше 255", withRow(good, 7, {"B4", "BYTE", "300", ""}), defaultOps(), 1},
        {"Ошибка: X\"...\" с нечётным числом цифр", withRow(good, 5, {"B2", "BYTE", "X\"2F4C008\"", ""}), defaultOps(), 1},
        {"Ошибка в ТКО: код занят дважды", good, badOps, 1},
        {"Несколько ошибок сразу (1-й проход)", manyErrors, defaultOps(), 1},
        {"Ошибка 2-го прохода: неопределённая метка", withRow(good, 11, {"", "SAVER1", "B5", ""}), defaultOps(), 2},
        {"Несколько ошибок сразу (2-й проход)", manyUndefined, defaultOps(), 2},
    };
}
