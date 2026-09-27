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

// Программа, в которой есть все 4 формата команд. Только прямая адресация.
Table referenceSource() {
    return {
        {"Exampl", "Start", "0", ""},
        {"", "LD", "R1", "One"},   // формат 4
        {"", "LD", "R2", "Two"},   // формат 4
        {"", "ADD", "R1", "R2"},   // формат 2, регистры
        {"", "SAV", "R1", "Rez"},  // формат 4
        {"", "CALL", "Show", ""},  // формат 3
        {"", "HALT", "", ""},      // формат 1
        {"Show", "INT", "5", ""},  // формат 2, число
        {"", "RET", "", ""},       // формат 1
        {"One", "WORD", "1", ""},
        {"Two", "WORD", "2", ""},
        {"Rez", "WORD", "?", ""},
        {"Text", "BYTE", "\"Hello\"", ""},
        {"", "End", "", ""},
    };
}

// Копия таблицы, в которой строка index заменена на row.
Table withRow(Table table, int index, Row row) {
    table[index] = row;
    return table;
}

// Та же программа, но все метки в операндах в скобках - только относительная адресация.
Table relativeSource() {
    Table source = referenceSource();
    source[1] = {"", "LD", "R1", "[One]"};
    source[2] = {"", "LD", "R2", "[Two]"};
    source[4] = {"", "SAV", "R1", "[Rez]"};
    source[5] = {"", "CALL", "[Show]", ""};
    return source;
}

// Смешанная адресация: часть меток в скобках, часть без.
// JUMP [Loop] - переход назад, смещение получается отрицательным.
Table mixedSource() {
    Table source = referenceSource();
    source[1] = {"Loop", "LD", "R1", "One"};
    source[2] = {"", "LD", "R2", "[Two]"};
    source[4] = {"", "SAV", "R1", "[Rez]"};
    source[6] = {"", "JUMP", "[Loop]", ""};
    return source;
}

vector<Example> allExamples() {
    Table good = referenceSource();

    Table noEnd = good;
    noEnd.pop_back();

    Table badOps = defaultOps();
    badOps[1] = {"SAV", "01", "4"};

    // Несколько ошибок первого прохода в одной программе - все показываются сразу.
    Table manyErrors = withRow(good, 2, {"", "LDD", "R2", "Two"});
    manyErrors = withRow(manyErrors, 10, {"One", "WORD", "2", ""});
    manyErrors = withRow(manyErrors, 4, {"", "SAV", "R1", "[Rez"});

    // Две опечатки в метках (одна с прямой, одна с относительной адресацией):
    // первый проход проходит, второй находит обе.
    Table manyUndefined = withRow(mixedSource(), 1, {"Loop", "LD", "R1", "Onw"});
    manyUndefined = withRow(manyUndefined, 4, {"", "SAV", "R1", "[Res]"});

    return {
        {"Только прямая адресация", good, defaultOps(), 0},
        {"Только относительная адресация", relativeSource(), defaultOps(), 0},
        {"Смешанная адресация", mixedSource(), defaultOps(), 0},
        {"Ошибка: адрес в Start не 0", withRow(good, 0, {"Exampl", "Start", "1000", ""}), defaultOps(), 1},
        {"Ошибка: [метка] у команды без адреса", withRow(good, 3, {"", "ADD", "R1", "[R2]"}), defaultOps(), 1},
        {"Ошибка: не закрыта скобка [метки", withRow(good, 4, {"", "SAV", "R1", "[Rez"}), defaultOps(), 1},
        {"Ошибка: метка определена дважды", withRow(good, 10, {"One", "WORD", "2", ""}), defaultOps(), 1},
        {"Ошибка: метка — зарезервированное слово", withRow(good, 9, {"ADD", "WORD", "1", ""}), defaultOps(), 1},
        {"Ошибка: неизвестная команда", withRow(good, 2, {"", "LDD", "R2", "Two"}), defaultOps(), 1},
        {"Ошибка: операнды не подходят формату", withRow(good, 3, {"", "ADD", "R1", "One"}), defaultOps(), 1},
        {"Ошибка: нет директивы End", noEnd, defaultOps(), 1},
        {"Ошибка: неверное значение BYTE", withRow(good, 12, {"Text", "BYTE", "300", ""}), defaultOps(), 1},
        {"Ошибка в ТКО: код занят дважды", good, badOps, 1},
        {"Несколько ошибок сразу (1-й проход)", manyErrors, defaultOps(), 1},
        {"Ошибка 2-го прохода: неопределённая метка [Res]", withRow(relativeSource(), 4, {"", "SAV", "R1", "[Res]"}), defaultOps(), 2},
        {"Несколько ошибок сразу (2-й проход)", manyUndefined, defaultOps(), 2},
    };
}
