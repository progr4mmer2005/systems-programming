#pragma once

#include <string>

using namespace std;

#include "../util/text.h"
#include "names.h"

string expectedOperands(int format) {
    if (format == 1) return "без операндов";
    if (format == 2) return "<регистр> <регистр> или <число 0..255>";
    if (format == 3) return "<метка> или [<метка>] (адрес - 2 байта)";
    if (format == 4) return "<метка> или [<метка>] (адрес - 3 байта)";
    return "?";
}

bool isLabel(string s) { return isName(s) && !isRegister(s); }

bool isRelativeLabel(string s) {
    if (len(s) < 3 || s[0] != '[' || s[len(s) - 1] != ']') return false;
    return isLabel(s.substr(1, len(s) - 2));
}

string withoutBrackets(string s) {
    if (isRelativeLabel(s)) return s.substr(1, len(s) - 2);
    return s;
}

Number parseSmallNumber(string s) {
    Number n = parseCount(s);
    if (n.value > 255) n.ok = false;
    return n;
}

string operandKind(int format, string a, string b) {
    if (format == 1 && a == "" && b == "") return "none";
    if (format == 2 && isRegister(a) && isRegister(b)) return "registers";
    if (format == 2 && parseSmallNumber(a).ok && b == "") return "number";
    if ((format == 3 || format == 4) && isLabel(a) && b == "") return "label";
    if ((format == 3 || format == 4) && isRelativeLabel(a) && b == "") return "relative";
    return "";
}

string joinOperands(string a, string b) {
    if (a == "" && b == "") return "(пусто)";
    if (b == "") return a;
    if (a == "") return "_ " + b;
    return a + " " + b;
}

const int MAX_DATA_BYTES = 30;

struct ByteString {
    bool isString = false;
    string problem;
    string code;
    int size = 0;
};

ByteString parseByteString(string s) {
    ByteString result;
    string prefix = upper(s.substr(0, 2));
    int skip = 0;
    if (prefix == "C\"" || prefix == "X\"")
        skip = 2;
    else if (s != "" && s[0] == '"')
        skip = 1;
    else
        return result;
    result.isString = true;
    if (len(s) < skip + 1 || s[len(s) - 1] != '"') {
        result.problem = "не закрыта кавычка - строка должна заканчиваться символом \"";
        return result;
    }
    string body = s.substr(skip, len(s) - skip - 1);
    if (body == "") {
        result.problem = "пустая строка";
        return result;
    }

    if (prefix != "X\"") {
        for (char c : body) {
            int code = (unsigned char)c;
            if (code < 0x20 || code > 0x7E) {
                result.problem = "в строке допустимы только латинские буквы, цифры и знаки (ASCII)";
                return result;
            }
            result.code += hex(code, 2);
        }
        result.size = len(body);
    } else {
        if (!parseHex(body, len(body)).ok) {
            result.problem = "в X\"...\" допустимы только шестнадцатеричные цифры 0-9, A-F";
            return result;
        }
        if (len(body) % 2 != 0) {
            result.problem = "в X\"...\" нечётное число цифр - на байт нужно ровно 2 цифры";
            return result;
        }
        result.code = upper(body);
        result.size = len(body) / 2;
    }
    if (result.size > MAX_DATA_BYTES) result.problem = "строка длиннее " + to_string(MAX_DATA_BYTES) + " байт";
    return result;
}
