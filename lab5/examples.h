#pragma once

#include <string>
#include <vector>

using namespace std;

#include "asm/encoding.h"
#include "util/table.h"

struct Example {
    string title;
    Table source;
    Table ops;
    int mode;
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

Table directSource() {
    return {
        {"PROG", "START", "0", ""},
        {"", "JMP", "L1", ""},
        {"A1", "RESB", "10", ""},
        {"A2", "RESW", "20", ""},
        {"B1", "WORD", "4096", ""},
        {"B2", "BYTE", "X\"2F4C008A\"", ""},
        {"B3", "BYTE", "C\"Hello!\"", ""},
        {"B4", "BYTE", "128", ""},
        {"L1", "LOADR1", "B1", ""},
        {"", "LOADR2", "B4", ""},
        {"", "ADD", "R1", "R2"},
        {"", "SAVER1", "L1", ""},
        {"", "INT", "200", ""},
        {"", "END", "10", ""},
    };
}

Table relativeSource() {
    Table source = directSource();
    source[1] = {"", "JMP", "[L1]", ""};
    source[8] = {"L1", "LOADR1", "[B1]", ""};
    source[9] = {"", "LOADR2", "[B4]", ""};
    source[11] = {"", "SAVER1", "[L1]", ""};
    return source;
}

Table mixedSource() {
    Table source = directSource();
    source[1] = {"", "JMP", "[L1]", ""};
    source[9] = {"", "LOADR2", "[B4]", ""};
    source[11] = {"", "SAVER1", "[L1]", ""};
    return source;
}

Table allFormatsSource() {
    return {
        {"PROG", "START", "0", ""},
        {"", "LOADR1", "One", ""},
        {"", "LOADR2", "[Two]", ""},
        {"", "ADD", "R1", "R2"},
        {"", "SAVER1", "[Rez]", ""},
        {"", "CALL", "[Show]", ""},
        {"", "NOP", "", ""},
        {"Show", "INT", "5", ""},
        {"", "CALL", "Exit", ""},
        {"One", "WORD", "1", ""},
        {"Two", "WORD", "2", ""},
        {"Rez", "WORD", "?", ""},
        {"Text", "BYTE", "C\"Hello\"", ""},
        {"Exit", "NOP", "", ""},
        {"", "END", "", ""},
    };
}

Table farJumpSource() {
    return {
        {"PROG", "START", "0", ""},
        {"", "CALL", "[Far]", ""},
        {"Buf", "RESB", "40000", ""},
        {"Far", "NOP", "", ""},
        {"", "END", "", ""},
    };
}

Table sharedLabelSource() {
    return {
        {"PROG", "START", "0", ""},
        {"", "LOADR1", "X", ""},
        {"", "LOADR2", "[X]", ""},
        {"", "ADD", "R1", "R2"},
        {"", "SAVER1", "X", ""},
        {"", "JMP", "[Fin]", ""},
        {"X", "WORD", "5", ""},
        {"Fin", "NOP", "", ""},
        {"", "END", "", ""},
    };
}

Table backwardSource() {
    return {
        {"PROG", "START", "0", ""},
        {"One", "WORD", "1", ""},
        {"Loop", "LOADR1", "[One]", ""},
        {"", "LOADR2", "One", ""},
        {"", "ADD", "R1", "R2"},
        {"", "SAVER1", "[One]", ""},
        {"", "JMP", "[Loop]", ""},
        {"", "END", "Loop", ""},
    };
}

Table withRow(Table table, int index, Row row) {
    table[index] = row;
    return table;
}

vector<Example> allExamples() {
    Table good = directSource();

    Table noEnd = good;
    noEnd.pop_back();

    Table badOps = defaultOps();
    badOps[1] = {"LOADR1", "01", "4"};

    Table manyErrors = withRow(mixedSource(), 9, {"", "LOADR3", "[B4]", ""});
    manyErrors = withRow(manyErrors, 7, {"B1", "BYTE", "128", ""});
    manyErrors = withRow(manyErrors, 11, {"", "SAVER1", "[L1", ""});

    Table manyUndefined = withRow(mixedSource(), 8, {"L1", "LOADR1", "B5", ""});
    manyUndefined = withRow(manyUndefined, 11, {"", "SAVER1", "[L2]", ""});

    return {
        {"Без ошибок: только прямая адресация", good, defaultOps(), MODE_DIRECT},
        {"Без ошибок: только относительная адресация", relativeSource(), defaultOps(), MODE_RELATIVE},
        {"Без ошибок: смешанная адресация", mixedSource(), defaultOps(), MODE_BOTH},
        {"Без ошибок: все 4 формата команд", allFormatsSource(), defaultOps(), MODE_BOTH},
        {"Без ошибок: одну метку ждут несколько команд", sharedLabelSource(), defaultOps(), MODE_BOTH},
        {"Без ошибок: отрицательное смещение", backwardSource(), defaultOps(), MODE_BOTH},
        {"Без ошибок: точка входа - метка (END L1)", withRow(good, 13, {"", "END", "L1", ""}), defaultOps(), MODE_DIRECT},
        {"Ошибка: [метка] в режиме «только прямая»", withRow(good, 1, {"", "JMP", "[L1]", ""}), defaultOps(), MODE_DIRECT},
        {"Ошибка: метка без скобок в режиме «только относительная»", withRow(relativeSource(), 9, {"", "LOADR2", "B4", ""}),
         defaultOps(), MODE_RELATIVE},
        {"Ошибка: адрес в START не 0", withRow(good, 0, {"PROG", "START", "100", ""}), defaultOps(), MODE_DIRECT},
        {"Ошибка: [метка] у команды без адреса", withRow(mixedSource(), 12, {"", "INT", "[B4]", ""}), defaultOps(), MODE_BOTH},
        {"Ошибка: не закрыта скобка [метки", withRow(mixedSource(), 9, {"", "LOADR2", "[B4", ""}), defaultOps(), MODE_BOTH},
        {"Ошибка: метка определена дважды", withRow(good, 7, {"B1", "BYTE", "128", ""}), defaultOps(), MODE_DIRECT},
        {"Ошибка: метка - зарезервированное слово", withRow(good, 4, {"ADD", "WORD", "4096", ""}), defaultOps(), MODE_DIRECT},
        {"Ошибка: неизвестная команда", withRow(good, 9, {"", "LOADR3", "B4", ""}), defaultOps(), MODE_DIRECT},
        {"Ошибка: нет директивы END", noEnd, defaultOps(), MODE_DIRECT},
        {"Ошибка: неверная точка входа", withRow(good, 13, {"", "END", "100", ""}), defaultOps(), MODE_DIRECT},
        {"Ошибка: метки точки входа нет (END L9)", withRow(good, 13, {"", "END", "L9", ""}), defaultOps(), MODE_DIRECT},
        {"Ошибка: метка на строке END", withRow(good, 13, {"FIN", "END", "10", ""}), defaultOps(), MODE_DIRECT},
        {"Ошибка в ТКО: код занят дважды", good, badOps, MODE_DIRECT},
        {"Несколько ошибок сразу", manyErrors, defaultOps(), MODE_BOTH},
        {"Ошибка: метка [B5] не определена (видно только на End)", withRow(relativeSource(), 9, {"", "LOADR2", "[B5]", ""}), defaultOps(),
         MODE_RELATIVE},
        {"Ошибка: смещение не помещается в 2 байта", farJumpSource(), defaultOps(), MODE_BOTH},
        {"Ошибка: несколько неопределённых меток", manyUndefined, defaultOps(), MODE_BOTH},
    };
}
