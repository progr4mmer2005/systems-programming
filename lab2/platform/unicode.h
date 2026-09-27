// Весь текст в программе - обычный string (UTF-8).
// Windows работает с UTF-16 (wstring), эти функции переводят
// из одного в другое при вызовах Windows.
#pragma once

#include <string>

using namespace std;

#include "winapi.h"

wstring toWide(string text) {
    if (text == "") return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), (int)text.size(), NULL, 0);
    wstring result(size, L' ');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), (int)text.size(), &result[0], size);
    return result;
}

string fromWide(wstring text) {
    if (text == L"") return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(), NULL, 0, NULL, NULL);
    string result(size, ' ');
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(), &result[0], size, NULL, NULL);
    return result;
}

// Текст из поля ввода Windows.
string windowText(HWND window) {
    wchar_t buffer[512];
    GetWindowTextW(window, buffer, 512);
    return fromWide(buffer);
}

void setWindowText(HWND window, string text) { SetWindowTextW(window, toWide(text).c_str()); }
