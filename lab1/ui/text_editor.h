// Окно "Редактировать как текст": большое поле ввода и кнопки "Применить" / "Отмена".
// Пока оно открыто, главное окно недоступно.
#pragma once

#include <string>

using namespace std;

#include "../platform/unicode.h"
#include "button.h"
#include "draw.h"
#include "fonts.h"
#include "metrics.h"
#include "theme.h"

const int ID_EDITOR_OK = 300;
const int ID_EDITOR_CANCEL = 301;

struct TextEditorResult {
    bool ok;      // нажали "Применить"
    string text;  // текст из поля
};

HWND textEditorWindow = NULL;
HWND textEditorBox = NULL;
HWND textEditorOk = NULL;
HWND textEditorCancel = NULL;
WNDPROC textEditorBoxOldProc = NULL;
string textEditorHint;
bool textEditorClosed = false;
bool textEditorAccepted = false;

void closeTextEditor(bool accept) {
    textEditorAccepted = accept;
    textEditorClosed = true;
}

// Расположение: подсказка сверху, поле ввода, кнопки снизу справа.
void layoutTextEditor() {
    RECT area;
    GetClientRect(textEditorWindow, &area);
    int pad = S(16);
    int buttonWidth = S(120), buttonHeight = S(34);
    int bottom = area.bottom - pad - buttonHeight;
    MoveWindow(textEditorBox, pad + 1, pad + S(40) + 1, area.right - 2 * pad - 2, bottom - pad - (pad + S(40)) - 2, TRUE);
    MoveWindow(textEditorCancel, area.right - pad - buttonWidth, bottom, buttonWidth, buttonHeight, TRUE);
    MoveWindow(textEditorOk, area.right - pad - 2 * buttonWidth - S(10), bottom, buttonWidth, buttonHeight, TRUE);
    InvalidateRect(textEditorWindow, NULL, FALSE);
}

void paintTextEditor(HWND window) {
    PAINTSTRUCT paint;
    HDC canvas = BeginPaint(window, &paint);
    RECT area;
    GetClientRect(window, &area);
    fill(canvas, area, colors.window);

    int pad = S(16);
    RECT hint = {pad, pad, area.right - pad, pad + S(36)};
    drawText(canvas, textEditorHint, hint, fonts.hint, colors.muted, DT_LEFT | DT_TOP | DT_WORDBREAK);

    RECT border;
    GetWindowRect(textEditorBox, &border);
    MapWindowPoints(NULL, window, (POINT*)&border, 2);
    InflateRect(&border, 1, 1);
    frame(canvas, border, colors.border);
    EndPaint(window, &paint);
}

LRESULT CALLBACK textEditorProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_SIZE) {
        if (textEditorBox) layoutTextEditor();
        return 0;
    }
    if (message == WM_GETMINMAXINFO) {
        MINMAXINFO* limits = (MINMAXINFO*)lParam;
        limits->ptMinTrackSize.x = S(520);
        limits->ptMinTrackSize.y = S(360);
        return 0;
    }
    if (message == WM_ERASEBKGND) return 1;
    if (message == WM_PAINT) {
        paintTextEditor(window);
        return 0;
    }
    if (message == WM_DRAWITEM) {
        drawButton((DRAWITEMSTRUCT*)lParam);
        return TRUE;
    }
    if (message == WM_CTLCOLOREDIT) {
        gdi.SetTextColor((HDC)wParam, colors.ink);
        gdi.SetBkColor((HDC)wParam, colors.card);
        return (LRESULT)inputBrush;
    }
    if (message == WM_COMMAND) {
        if (LOWORD(wParam) == ID_EDITOR_OK) closeTextEditor(true);
        if (LOWORD(wParam) == ID_EDITOR_CANCEL) closeTextEditor(false);
        return 0;
    }
    if (message == WM_CLOSE) {
        closeTextEditor(false);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

// Клавиши в поле ввода: Ctrl+A - выделить всё, Ctrl+Enter - применить, Esc - отмена.
LRESULT CALLBACK textEditorBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    bool ctrl = GetKeyState(VK_CONTROL) < 0;
    if (message == WM_KEYDOWN) {
        if (ctrl && wParam == 'A') {
            SendMessageW(hwnd, EM_SETSEL, 0, -1);
            return 0;
        }
        if (ctrl && wParam == VK_RETURN) {
            closeTextEditor(true);
            return 0;
        }
        if (wParam == VK_ESCAPE) {
            closeTextEditor(false);
            return 0;
        }
    }
    // Символы, которые дают эти сочетания клавиш, в текст не вставляем.
    if (message == WM_CHAR && (wParam == 1 || wParam == 10 || wParam == 27)) return 0;
    return CallWindowProcW(textEditorBoxOldProc, hwnd, message, wParam, lParam);
}

// Windows в поле ввода хочет переводы строк "\r\n".
string withWindowsNewlines(string text) {
    string result;
    for (char c : text) {
        if (c == '\n') result += '\r';
        if (c != '\r') result += c;
    }
    return result;
}

string withoutCarriageReturns(string text) {
    string result;
    for (char c : text) {
        if (c != '\r') result += c;
    }
    return result;
}

TextEditorResult openTextEditor(HWND owner, string title, string hint, string text) {
    static bool registered = false;
    if (!registered) {
        WNDCLASSEXW windowClass = {};
        windowClass.cbSize = sizeof(windowClass);
        windowClass.lpfnWndProc = textEditorProc;
        windowClass.hInstance = GetModuleHandleW(NULL);
        windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
        windowClass.lpszClassName = L"AsmTextEditor";
        RegisterClassExW(&windowClass);
        registered = true;
    }

    textEditorHint = hint;
    textEditorClosed = false;
    textEditorAccepted = false;

    // Окно по центру главного окна.
    RECT ownerBox;
    GetWindowRect(owner, &ownerBox);
    int width = S(760), height = S(620);
    int x = (ownerBox.left + ownerBox.right - width) / 2;
    int y = (ownerBox.top + ownerBox.bottom - height) / 2;
    textEditorWindow = CreateWindowExW(WS_EX_DLGMODALFRAME, L"AsmTextEditor", toWide(title).c_str(),
                                       WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_CLIPCHILDREN, x, y, width,
                                       height, owner, NULL, GetModuleHandleW(NULL), NULL);
    applyTitleBarTheme(textEditorWindow);

    textEditorBox = CreateWindowExW(0, L"EDIT", L"",
                                    WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE |
                                        ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN,
                                    0, 0, 10, 10, textEditorWindow, NULL, GetModuleHandleW(NULL), NULL);
    SendMessageW(textEditorBox, WM_SETFONT, (WPARAM)fonts.mono, TRUE);
    SendMessageW(textEditorBox, EM_SETLIMITTEXT, 0, 0);
    SendMessageW(textEditorBox, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(S(6), S(6)));
    applyControlTheme(textEditorBox, L"DarkMode_Explorer");
    setWindowText(textEditorBox, withWindowsNewlines(text));
    textEditorBoxOldProc = (WNDPROC)SetWindowLongPtrW(textEditorBox, GWLP_WNDPROC, (LONG_PTR)textEditorBoxProc);

    textEditorOk = makeButton(textEditorWindow, ID_EDITOR_OK, "Применить", BUTTON_PRIMARY);
    textEditorCancel = makeButton(textEditorWindow, ID_EDITOR_CANCEL, "Отмена", BUTTON_SECONDARY);

    layoutTextEditor();
    EnableWindow(owner, FALSE);
    ShowWindow(textEditorWindow, SW_SHOW);
    SetFocus(textEditorBox);

    // Свой цикл сообщений, пока окно не закроют.
    MSG message;
    while (!textEditorClosed) {
        int got = GetMessageW(&message, NULL, 0, 0);
        if (got <= 0) {
            PostQuitMessage((int)message.wParam);  // программу закрывают - вернуть сообщение главному циклу
            break;
        }
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    TextEditorResult result = {textEditorAccepted, withoutCarriageReturns(windowText(textEditorBox))};

    // Сначала включить главное окно, иначе после закрытия фокус уйдёт в другую программу.
    EnableWindow(owner, TRUE);
    DestroyWindow(textEditorWindow);
    for (int i = len(buttons) - 1; i >= 0; i--) {
        if (buttons[i].hwnd == textEditorOk || buttons[i].hwnd == textEditorCancel) buttons.erase(buttons.begin() + i);
    }
    textEditorWindow = NULL;
    textEditorBox = NULL;
    SetForegroundWindow(owner);
    return result;
}
