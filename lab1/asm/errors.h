// Ошибки, которые находит ассемблер.
#pragma once

#include <string>

using namespace std;

struct Error {
    string place;  // "source" - исходный текст, "ops" - ТКО, "program" - программа целиком
    int row;       // номер строки в таблице (с нуля); -1 - ошибка не про одну строку
    string text;
};

// Для столбца "Где": "стр. 3", "ТКО, стр. 2" или "—".
string placeText(Error error) {
    if (error.place == "source") return "стр. " + to_string(error.row + 1);
    if (error.place == "ops") return "ТКО, стр. " + to_string(error.row + 1);
    return "—";
}
