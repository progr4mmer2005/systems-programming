#pragma once

#include <string>

using namespace std;

#include "../platform/unicode.h"
#include "fonts.h"

HWND makeButton(HWND parent, int id, string caption) {
    HWND hwnd = CreateWindowExW(0, L"BUTTON", toWide(caption).c_str(), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 0, 0,
                                10, 10, parent, (HMENU)(INT_PTR)id, GetModuleHandleW(NULL), NULL);
    SendMessageW(hwnd, WM_SETFONT, (WPARAM)fonts.normal, TRUE);
    return hwnd;
}
