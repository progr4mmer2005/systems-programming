#pragma once

#include <string>
#include <vector>

using namespace std;

#include "util/table.h"

struct Example {
    string title;
    Table source;
    Table ops;
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

Table backwardSource() {
    return {
        {"PROG", "START", "100", ""},
        {"B1", "WORD", "40", ""},
        {"B4", "BYTE", "12", ""},
        {"L1", "LOADR1", "B1", ""},
        {"", "LOADR2", "B4", ""},
        {"", "ADD", "R1", "R2"},
        {"", "SAVER1", "B1", ""},
        {"", "JMP", "L1", ""},
        {"", "END", "L1", ""},
    };
}

Table sharedLabelSource() {
    return {
        {"PROG", "START", "2000", ""},
        {"", "LOADR1", "X", ""},
        {"", "LOADR2", "X", ""},
        {"", "ADD", "R1", "R2"},
        {"", "SAVER1", "X", ""},
        {"", "JMP", "Fin", ""},
        {"X", "WORD", "5", ""},
        {"Fin", "NOP", "", ""},
        {"", "END", "", ""},
    };
}

Table farLabelSource() {
    return {
        {"PROG", "START", "FFF0", ""},
        {"", "CALL", "Far", ""},
        {"", "NOP", "", ""},
        {"Buf", "RESB", "20", ""},
        {"Far", "NOP", "", ""},
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
        {"Без ошибок", good, defaultOps()},
        {"Без ошибок: все 4 формата команд, много ссылок вперёд", allFormatsSource(), defaultOps()},
        {"Без ошибок: метки определены раньше, чем используются", backwardSource(), defaultOps()},
        {"Без ошибок: одну метку ждут несколько команд", sharedLabelSource(), defaultOps()},
        {"Без ошибок: точка входа - метка (END L1)", withRow(good, 13, {"", "END", "L1", ""}), defaultOps()},
        {"Ошибка: метка определена дважды", withRow(good, 7, {"B1", "BYTE", "12", ""}), defaultOps()},
        {"Ошибка: метка - зарезервированное слово", withRow(good, 4, {"ADD", "WORD", "40", ""}), defaultOps()},
        {"Ошибка: неизвестная команда", withRow(good, 9, {"", "LOADR3", "B4", ""}), defaultOps()},
        {"Ошибка: операнды не подходят формату", withRow(good, 10, {"", "ADD", "R1", "B1"}), defaultOps()},
        {"Ошибка: неверный адрес загрузки", withRow(good, 0, {"PROG", "START", "10G", ""}), defaultOps()},
        {"Ошибка: нет директивы END", noEnd, defaultOps()},
        {"Ошибка: неверная точка входа", withRow(good, 13, {"", "END", "50", ""}), defaultOps()},
        {"Ошибка: метки точки входа нет (END L9)", withRow(good, 13, {"", "END", "L9", ""}), defaultOps()},
        {"Ошибка: метка на строке END", withRow(good, 13, {"FIN", "END", "100", ""}), defaultOps()},
        {"Ошибка: относительная адресация [L1]", withRow(good, 1, {"", "JMP", "[L1]", ""}), defaultOps()},
        {"Ошибка: BYTE больше 255", withRow(good, 7, {"B4", "BYTE", "300", ""}), defaultOps()},
        {"Ошибка: X\"...\" с нечётным числом цифр", withRow(good, 5, {"B2", "BYTE", "X\"2F4C008\"", ""}), defaultOps()},
        {"Ошибка в ТКО: код занят дважды", good, badOps},
        {"Ошибка: метка не определена (видно только на End)", withRow(good, 11, {"", "SAVER1", "B5", ""}), defaultOps()},
        {"Ошибка: несколько неопределённых меток", manyUndefined, defaultOps()},
        {"Ошибка: адрес метки не влез в 2 байта (CALL)", farLabelSource(), defaultOps()},
        {"Несколько ошибок сразу", manyErrors, defaultOps()},
    };
}
