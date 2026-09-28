// Разбор операндов команды и данных BYTE.
#pragma once

#include <string>
#include <vector>

using namespace std;

#include "../util/text.h"
#include "names.h"

// Что должно стоять в операндах команды каждого формата - для текста ошибки.
string expectedOperands(int format) {
    if (format == 1) return "без операндов";
    if (format == 2) return "<регистр> <регистр> или <число 0..255>";
    if (format == 3) return "<метка> или [<метка>]";
    if (format == 4) return "<регистр> <метка> или <регистр> [<метка>]";
    return "?";
}

// Метка в операнде: имя, которое не регистр.
bool isLabel(string s) { return isName(s) && !isRegister(s); }

// Метка в квадратных скобках [One] - относительная адресация.
bool isRelativeLabel(string s) {
    if (len(s) < 3 || s[0] != '[' || s[len(s) - 1] != ']') return false;
    return isLabel(s.substr(1, len(s) - 2));
}

// "[One]" -> "One", "One" -> "One"
string withoutBrackets(string s) {
    if (isRelativeLabel(s)) return s.substr(1, len(s) - 2);
    return s;
}

// Число 0..255 для формата 2.
Number parseSmallNumber(string s) {
    Number n = parseDecimal(s);
    if (n.value < 0 || n.value > 255) n.ok = false;
    return n;
}

// Что стоит в операндах a, b команды данного формата:
//   "none", "registers", "number", "label", "register+label".
// "" - операнды не подходят формату.
string operandKind(int format, string a, string b) {
    bool aIsLabel = isLabel(a) || isRelativeLabel(a);
    bool bIsLabel = isLabel(b) || isRelativeLabel(b);
    if (format == 1 && a == "" && b == "") return "none";
    if (format == 2 && isRegister(a) && isRegister(b)) return "registers";
    if (format == 2 && parseSmallNumber(a).ok && b == "") return "number";
    if (format == 3 && aIsLabel && b == "") return "label";
    if (format == 4 && isRegister(a) && bIsLabel) return "register+label";
    return "";
}

// Операнды для текста ошибки: "R1 One", "(пусто)".
string joinOperands(string a, string b) {
    if (a == "" && b == "") return "(пусто)";
    if (b == "") return a;
    if (a == "") return "_ " + b;
    return a + " " + b;
}

// Разбор строки BYTE "текст". Строка - всё между первой и последней кавычкой,
// кавычки внутри - обычные символы: "Hello"" -> Hello"
struct TextBytes {
    bool isText = false;  // операнд - строка в кавычках?
    string problem;       // что не так ("" - всё хорошо)
    vector<int> bytes;    // коды символов
};

TextBytes parseText(string s) {
    TextBytes result;
    if (len(s) < 2 || s[0] != '"' || s[len(s) - 1] != '"') return result;
    result.isText = true;
    string body = s.substr(1, len(s) - 2);
    if (body == "") result.problem = "пустая строка";
    for (char c : body) {
        int code = (unsigned char)c;
        if (code < 0x20 || code > 0x7E) {
            result.problem = "допустимы только латинские буквы, цифры и знаки (ASCII)";
            break;
        }
        result.bytes.push_back(code);
    }
    return result;
}

// [72, 101] -> "48 65"
string bytesToHex(vector<int> bytes) {
    string result = "";
    for (int i = 0; i < len(bytes); i++) {
        if (i > 0) result += " ";
        result += hex(bytes[i], 2);
    }
    return result;
}
