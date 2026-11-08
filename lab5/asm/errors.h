#pragma once

#include <string>

using namespace std;

struct Error {
    string place;
    int row;
    string text;
};

string placeText(Error error) {
    if (error.place == "source") return "стр. " + to_string(error.row + 1);
    if (error.place == "ops") return "ТКО, стр. " + to_string(error.row + 1);
    return "—";
}
