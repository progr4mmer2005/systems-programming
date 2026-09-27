// Таблица кодов операций (ТКО): проверка и поиск команды.
#pragma once

#include <string>
#include <vector>

using namespace std;

#include "../util/table.h"
#include "encoding.h"
#include "errors.h"
#include "names.h"

struct OpCode {
    string name;  // мнемоника, большими буквами
    int code;     // 00..3F
    int length;   // длина в байтах = формат команды, 1..4
    int row;      // строка в таблице ТКО
};

// Номер команды в списке или -1, если такой нет.
int findOp(vector<OpCode> ops, string name) {
    for (int i = 0; i < len(ops); i++) {
        if (sameName(ops[i].name, name)) return i;
    }
    return -1;
}

// Проверить одну строку ТКО. "" - всё хорошо, иначе текст ошибки.
string checkOpRow(string name, string codeText, string lengthText, vector<OpCode> ops) {
    if (name == "" || codeText == "" || lengthText == "") return "Заполнены не все ячейки: нужны мнемоника, код и длина";
    if (!isName(name)) return "Мнемоника «" + name + "»: только латинские буквы, цифры и «_», первая — буква";
    if (len(name) > MAX_NAME_LENGTH) return "Мнемоника «" + name + "» длиннее " + to_string(MAX_NAME_LENGTH) + " символов";
    if (isDirective(name)) return "Мнемоника «" + name + "» совпадает с директивой";
    if (isRegister(name)) return "Мнемоника «" + name + "» совпадает с именем регистра";

    Number code = parseHex(codeText, 2);
    if (!code.ok) return "Код «" + codeText + "»: нужно шестнадцатеричное число от 00 до 3F";
    if (code.value > MAX_OPCODE) return "Код «" + codeText + "» больше 3F: код занимает 6 бит, 2 младших бита первого байта — тип адресации";

    Number length = parseDecimal(lengthText);
    if (!length.ok || length.value < 1 || length.value > 4) return "Длина «" + lengthText + "»: формат команды — от 1 до 4 байт";

    for (OpCode other : ops) {
        if (sameName(other.name, name)) return "Команда " + upper(name) + " уже есть в строке " + to_string(other.row + 1);
        if (other.code == code.value)
            return "Код " + hex(code.value, 2) + " уже занят командой " + other.name + " (строка " + to_string(other.row + 1) + ")";
    }
    return "";
}

// Прочитать ТКО из таблицы окна: вернуть правильные команды, ошибки дописать в errors.
vector<OpCode> readOpTable(Table table, vector<Error>& errors) {
    vector<OpCode> ops;
    for (int row = 0; row < len(table); row++) {
        if (isEmptyRow(table[row])) continue;
        string name = cell(table[row], 0);
        string codeText = cell(table[row], 1);
        string lengthText = cell(table[row], 2);

        string problem = checkOpRow(name, codeText, lengthText, ops);
        if (problem != "") {
            errors.push_back({"ops", row, problem});
            continue;
        }
        int code = (int)parseHex(codeText, 2).value;
        int length = (int)parseDecimal(lengthText).value;
        ops.push_back({upper(name), code, length, row});
    }
    if (len(ops) == 0 && len(errors) == 0) errors.push_back({"ops", -1, "Таблица кодов операций пуста"});
    return ops;
}
