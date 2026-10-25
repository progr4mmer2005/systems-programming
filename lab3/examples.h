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
    int failsAt;
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

Table referenceSource() {
    return {
        {"first", "START", "0", ""},
        {"", "EXTDEF", "buf", "Str1"},
        {"", "EXTREF", "Str2", ""},
        {"Proc", "LOADR1", "Str3", ""},
        {"", "LOADR2", "Str2", ""},
        {"", "ADD", "R1", "R2"},
        {"", "SAVER1", "[Rez]", ""},
        {"", "JMP", "Proc", ""},
        {"Str1", "WORD", "3", ""},
        {"Str3", "WORD", "1", ""},
        {"Rez", "WORD", "?", ""},
        {"buf", "BYTE", "\"Hello!\"", ""},
        {"", "END", "", ""},
    };
}

Table withRow(Table table, int index, Row row) {
    table[index] = row;
    return table;
}

Table withInserted(Table table, int index, Row row) {
    table.insert(table.begin() + index, row);
    return table;
}

Table twoRefsSource() {
    Table source = withRow(referenceSource(), 2, {"", "EXTREF", "Str2", "Print"});
    return withInserted(source, 7, {"", "CALL", "Print", ""});
}

vector<Example> allExamples() {
    Table good = referenceSource();

    Table noEnd = good;
    noEnd.pop_back();

    Table lateRef = good;
    lateRef.erase(lateRef.begin() + 2);
    lateRef = withInserted(lateRef, len(lateRef) - 1, {"", "EXTREF", "Str2", ""});

    Table manyErrors = withRow(good, 2, {"", "EXTREF", "Str2", "Str1"});
    manyErrors = withRow(manyErrors, 5, {"", "ADDD", "R1", "R2"});
    manyErrors = withInserted(manyErrors, 12, {"Str2", "WORD", "5", ""});

    Table manyUndefined = withRow(good, 1, {"", "EXTDEF", "buf", "Str9"});
    manyUndefined = withRow(manyUndefined, 3, {"Proc", "LOADR1", "Str4", ""});
    manyUndefined = withRow(manyUndefined, 4, {"", "LOADR2", "[Str2]", ""});

    return {
        {"Без ошибок", good, defaultOps(), 0, MODE_BOTH},
        {"Две внешние ссылки", twoRefsSource(), defaultOps(), 0, MODE_BOTH},
        {"Ошибка: имя и в EXTDEF, и в EXTREF", withRow(good, 2, {"", "EXTREF", "Str2", "Str1"}), defaultOps(), 1, MODE_BOTH},
        {"Ошибка: метка совпадает с внешней ссылкой", withInserted(good, 12, {"Str2", "WORD", "5", ""}), defaultOps(), 1, MODE_BOTH},
        {"Ошибка: EXTREF не в начале программы", lateRef, defaultOps(), 1, MODE_BOTH},
        {"Ошибка: метка у EXTDEF", withRow(good, 1, {"Names", "EXTDEF", "buf", "Str1"}), defaultOps(), 1, MODE_BOTH},
        {"Ошибка: адрес в START не 0", withRow(good, 0, {"first", "START", "100", ""}), defaultOps(), 1, MODE_BOTH},
        {"Ошибка: метка определена дважды", withRow(good, 10, {"Str1", "WORD", "?", ""}), defaultOps(), 1, MODE_BOTH},
        {"Ошибка: метка - зарезервированное слово", withRow(good, 10, {"EXTREF", "WORD", "?", ""}), defaultOps(), 1, MODE_BOTH},
        {"Ошибка: неизвестная команда", withRow(good, 5, {"", "ADDD", "R1", "R2"}), defaultOps(), 1, MODE_BOTH},
        {"Ошибка: нет директивы END", noEnd, defaultOps(), 1, MODE_BOTH},
        {"Ошибка: точка входа - внешняя ссылка (END Str2)", withRow(good, 12, {"", "END", "Str2", ""}), defaultOps(), 1, MODE_BOTH},
        {"Несколько ошибок сразу (1-й проход)", manyErrors, defaultOps(), 1, MODE_BOTH},
        {"Ошибка 2-го прохода: имя из EXTDEF не определено", withRow(good, 1, {"", "EXTDEF", "buf", "Str9"}), defaultOps(), 2,
         MODE_BOTH},
        {"Ошибка 2-го прохода: внешняя ссылка [Str2]", withRow(good, 4, {"", "LOADR2", "[Str2]", ""}), defaultOps(), 2, MODE_BOTH},
        {"Ошибка 2-го прохода: неопределённая метка", withRow(good, 3, {"Proc", "LOADR1", "Str4", ""}), defaultOps(), 2, MODE_BOTH},
        {"Несколько ошибок сразу (2-й проход)", manyUndefined, defaultOps(), 2, MODE_BOTH},
    };
}
