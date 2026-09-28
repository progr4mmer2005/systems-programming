// Готовые примеры для списка "Пример": правильная программа и программы с ошибками.
#pragma once

#include <string>
#include <vector>

using namespace std;

#include "util/table.h"

struct Example {
    string title;
    Table source;
    Table ops;
    int failsAt;  // где пример должен "упасть": 0 - нигде, 1 - первый проход, 2 - второй
};

// ТКО по умолчанию: код - 6 бит (00..3F), длина = формат команды (1..4 байта).
Table defaultOps() {
    return {
        {"LD", "01", "4"},    // формат 4: LD  регистр, метка - загрузить слово в регистр
        {"SAV", "02", "4"},   // формат 4: SAV регистр, метка - сохранить регистр в память
        {"ADD", "03", "2"},   // формат 2: ADD регистр, регистр - сложить
        {"SUB", "04", "2"},   // формат 2: SUB регистр, регистр - вычесть
        {"JUMP", "05", "3"},  // формат 3: JUMP метка - переход
        {"CALL", "06", "3"},  // формат 3: CALL метка - вызов подпрограммы
        {"INT", "07", "2"},   // формат 2: INT число - прерывание
        {"RET", "08", "1"},   // формат 1: RET - возврат из подпрограммы
        {"HALT", "09", "1"},  // формат 1: HALT - остановка
    };
}

// Программа без ошибок: модуль отдаёт другим метки buf и Str1,
// а метку Str2 берёт из другого модуля. str3 и str2 в операндах написаны
// маленькими буквами - имена сравниваются без учёта регистра.
Table referenceSource() {
    return {
        {"first", "Start", "000000", ""},
        {"", "EXTDEF", "buf", "Str1"},
        {"", "EXTREF", "Str2", ""},
        {"Proc", "LD", "R1", "str3"},  // формат 4, прямая адресация
        {"", "LD", "R2", "str2"},      // формат 4, внешняя ссылка
        {"", "ADD", "R1", "R2"},       // формат 2
        {"", "SAV", "R1", "[Rez]"},    // формат 4, относительная адресация
        {"", "JUMP", "Proc", ""},      // формат 3, прямая адресация
        {"Str1", "WORD", "3", ""},
        {"Str3", "WORD", "1", ""},
        {"Rez", "WORD", "?", ""},
        {"buf", "BYTE", "\"Hello!\"", ""},
        {"", "End", "", ""},
    };
}

// Копия таблицы, в которой строка index заменена на row.
Table withRow(Table table, int index, Row row) {
    table[index] = row;
    return table;
}

// Копия таблицы, в которую перед строкой index вставлена row.
Table withInserted(Table table, int index, Row row) {
    table.insert(table.begin() + index, row);
    return table;
}

// Две внешние ссылки: Str2 и подпрограмма Print из другого модуля.
Table twoRefsSource() {
    Table source = withRow(referenceSource(), 2, {"", "EXTREF", "Str2", "Print"});
    return withInserted(source, 7, {"", "CALL", "Print", ""});
}

vector<Example> allExamples() {
    Table good = referenceSource();

    Table noEnd = good;
    noEnd.pop_back();

    // EXTREF после команд.
    Table lateRef = good;
    lateRef.erase(lateRef.begin() + 2);
    lateRef = withInserted(lateRef, len(lateRef) - 1, {"", "EXTREF", "Str2", ""});

    // Несколько ошибок первого прохода в одной программе - все показываются сразу.
    Table manyErrors = withRow(good, 2, {"", "EXTREF", "Str2", "Str1"});
    manyErrors = withRow(manyErrors, 5, {"", "ADDD", "R1", "R2"});
    manyErrors = withInserted(manyErrors, 12, {"Str2", "WORD", "5", ""});

    // Ошибки второго прохода: имя из EXTDEF не определено, внешняя ссылка
    // с относительной адресацией, опечатка в метке.
    Table manyUndefined = withRow(good, 1, {"", "EXTDEF", "buf", "Str9"});
    manyUndefined = withRow(manyUndefined, 3, {"Proc", "LD", "R1", "str4"});
    manyUndefined = withRow(manyUndefined, 4, {"", "LD", "R2", "[str2]"});

    return {
        {"Без ошибок", good, defaultOps(), 0},
        {"Две внешние ссылки", twoRefsSource(), defaultOps(), 0},
        {"Ошибка: имя и в EXTDEF, и в EXTREF", withRow(good, 2, {"", "EXTREF", "Str2", "Str1"}), defaultOps(), 1},
        {"Ошибка: метка совпадает с внешней ссылкой", withInserted(good, 12, {"Str2", "WORD", "5", ""}), defaultOps(), 1},
        {"Ошибка: EXTREF не в начале программы", lateRef, defaultOps(), 1},
        {"Ошибка: метка у EXTDEF", withRow(good, 1, {"Names", "EXTDEF", "buf", "Str1"}), defaultOps(), 1},
        {"Ошибка: адрес в Start не 0", withRow(good, 0, {"first", "Start", "001000", ""}), defaultOps(), 1},
        {"Ошибка: метка определена дважды", withRow(good, 10, {"Str1", "WORD", "?", ""}), defaultOps(), 1},
        {"Ошибка: метка — зарезервированное слово", withRow(good, 10, {"EXTREF", "WORD", "?", ""}), defaultOps(), 1},
        {"Ошибка: неизвестная команда", withRow(good, 5, {"", "ADDD", "R1", "R2"}), defaultOps(), 1},
        {"Ошибка: нет директивы End", noEnd, defaultOps(), 1},
        {"Несколько ошибок сразу (1-й проход)", manyErrors, defaultOps(), 1},
        {"Ошибка 2-го прохода: имя из EXTDEF не определено", withRow(good, 1, {"", "EXTDEF", "buf", "Str9"}), defaultOps(), 2},
        {"Ошибка 2-го прохода: внешняя ссылка [str2]", withRow(good, 4, {"", "LD", "R2", "[str2]"}), defaultOps(), 2},
        {"Ошибка 2-го прохода: неопределённая метка", withRow(good, 3, {"Proc", "LD", "R1", "str4"}), defaultOps(), 2},
        {"Несколько ошибок сразу (2-й проход)", manyUndefined, defaultOps(), 2},
    };
}
