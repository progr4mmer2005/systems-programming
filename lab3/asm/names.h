#pragma once

#include <string>

using namespace std;

#include "../util/text.h"

const int MAX_NAME_LENGTH = 10;
const int MAX_MNEMONIC_LENGTH = 6;

bool isLatinLetter(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }

bool isDigit(char c) { return c >= '0' && c <= '9'; }

bool isName(string s) {
    if (s == "" || !isLatinLetter(s[0])) return false;
    for (char c : s) {
        if (!isLatinLetter(c) && !isDigit(c) && c != '_') return false;
    }
    return true;
}

int registerNumber(string s) {
    s = upper(s);
    if (len(s) < 2 || len(s) > 3 || s[0] != 'R') return -1;
    string digits = s.substr(1);
    if (len(digits) == 2 && digits[0] == '0') return -1;
    Number n = parseDecimal(digits);
    if (!n.ok || digits[0] == '-' || digits[0] == '+' || n.value > 15) return -1;
    return (int)n.value;
}

bool isRegister(string s) { return registerNumber(s) != -1; }

bool isDirective(string s) {
    s = upper(s);
    return s == "START" || s == "END" || s == "WORD" || s == "BYTE" || s == "RESB" || s == "RESW" || s == "EXTDEF" ||
           s == "EXTNAME" || s == "EXTREF";
}

Number parseCount(string s) {
    Number n = parseDecimal(s);
    if (s == "" || !isDigit(s[0])) n.ok = false;
    return n;
}
