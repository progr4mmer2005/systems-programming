// Работа со строками и числами.
#pragma once

#include <string>
#include <vector>

using namespace std;

// Длина строки или списка.
int len(string s) { return (int)s.size(); }

template <typename T>
int len(vector<T> list) { return (int)list.size(); }

// "  LD " -> "LD"
string trim(string s) {
    int start = 0;
    int end = len(s);
    while (start < end && (s[start] == ' ' || s[start] == '\t')) start++;
    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t')) end--;
    return s.substr(start, end - start);
}

// "ld" -> "LD" (только латиница)
string upper(string s) {
    for (int i = 0; i < len(s); i++) {
        if (s[i] >= 'a' && s[i] <= 'z') s[i] = s[i] - 'a' + 'A';
    }
    return s;
}

// Имена сравниваются без учёта регистра: "Loop" и "LOOP" - одно и то же.
bool sameName(string a, string b) { return upper(a) == upper(b); }

// Число в hex-строку из digits цифр: hex(26, 4) = "001A"
string hex(long long value, int digits) {
    string symbols = "0123456789ABCDEF";
    string result = "";
    for (int i = 0; i < digits; i++) {
        result = symbols[value % 16] + result;
        value = value / 16;
    }
    return result;
}

// Результат разбора числа: ok - удалось ли разобрать, value - число.
struct Number {
    bool ok;
    long long value;
};

// "1A" -> 26. Цифр - от 1 до maxDigits.
Number parseHex(string s, int maxDigits) {
    Number result = {false, 0};
    if (s == "" || len(s) > maxDigits) return result;
    string symbols = "0123456789ABCDEF";
    for (char c : upper(s)) {
        int digit = (int)symbols.find(c);
        if (digit == -1) return result;
        result.value = result.value * 16 + digit;
    }
    result.ok = true;
    return result;
}

// "-12" -> -12. Не больше 12 цифр.
Number parseDecimal(string s) {
    Number result = {false, 0};
    bool negative = false;
    if (s != "" && (s[0] == '-' || s[0] == '+')) {
        negative = s[0] == '-';
        s = s.substr(1);
    }
    if (s == "" || len(s) > 12) return result;
    for (char c : s) {
        if (c < '0' || c > '9') return result;
        result.value = result.value * 10 + (c - '0');
    }
    if (negative) result.value = -result.value;
    result.ok = true;
    return result;
}

// Слово после числа: 1 ошибка, 2 ошибки, 5 ошибок.
string plural(long long n, string one, string few, string many) {
    long long last = n % 10, lastTwo = n % 100;
    if (last == 1 && lastTwo != 11) return one;
    if (last >= 2 && last <= 4 && (lastTwo < 12 || lastTwo > 14)) return few;
    return many;
}

// countText(3, "ошибка", "ошибки", "ошибок") = "3 ошибки"
string countText(long long n, string one, string few, string many) {
    return to_string(n) + " " + plural(n, one, few, many);
}

// Заменить первое вхождение what на with.
string replaceFirst(string s, string what, string with) {
    size_t at = s.find(what);
    if (at == string::npos) return s;
    return s.substr(0, at) + with + s.substr(at + what.size());
}
