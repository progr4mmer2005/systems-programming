// Имена в программе на ассемблере: метки, регистры, директивы.
#pragma once

#include <string>

using namespace std;

#include "../util/text.h"

const int MAX_NAME_LENGTH = 10;  // длина метки, имени программы, мнемоники

bool isLatinLetter(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }

bool isDigit(char c) { return c >= '0' && c <= '9'; }

// Имя: латинская буква, дальше латинские буквы, цифры или "_".
bool isName(string s) {
    if (s == "" || !isLatinLetter(s[0])) return false;
    for (char c : s) {
        if (!isLatinLetter(c) && !isDigit(c) && c != '_') return false;
    }
    return true;
}

// Регистр R0..R15 -> его номер. Не регистр -> -1.
int registerNumber(string s) {
    s = upper(s);
    if (len(s) < 2 || len(s) > 3 || s[0] != 'R') return -1;
    string digits = s.substr(1);
    if (len(digits) == 2 && digits[0] == '0') return -1;  // "R01" - не регистр
    Number n = parseDecimal(digits);
    if (!n.ok || digits[0] == '-' || digits[0] == '+' || n.value > 15) return -1;
    return (int)n.value;
}

bool isRegister(string s) { return registerNumber(s) != -1; }

// Директивы - указания ассемблеру, а не команды процессора.
// EXTNAME - то же, что EXTDEF (так она называется в Zasm из методички).
bool isDirective(string s) {
    s = upper(s);
    return s == "START" || s == "END" || s == "WORD" || s == "BYTE" || s == "EXTDEF" || s == "EXTNAME" || s == "EXTREF";
}
