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
    bool ok;
    string text;
};

HWND textEditorWindow = NULL;
HWND textEditorBox = NULL;
HWND textEditorOk = NULL;
HWND textEditorCancel = NULL;
WNDPROC textEditorBoxOldProc = NULL;
string textEditorHint;
bool textEditorClosed = false;
bool textEditorAccepted = false;

RECT textEditorGutter;
int textEditorLineHeight = 0;
int shownFirstLine = -1, shownLineCount = -1, shownCaretLine = -1;
bool askingEditor = false;

int askEditor(UINT message, WPARAM wParam, LPARAM lParam) {
    askingEditor = true;
    int answer = (int)SendMessageW(textEditorBox, message, wParam, lParam);
    askingEditor = false;
    return answer;
}

void closeTextEditor(bool accept) {
    textEditorAccepted = accept;
    textEditorClosed = true;
}

void layoutTextEditor() {
    RECT area;
    GetClientRect(textEditorWindow, &area);
    int pad = S(16);
    int buttonWidth = S(120), buttonHeight = S(34);
    int bottom = area.bottom - pad - buttonHeight;
    int gutterWidth = S(46);
    int top = pad + S(40) + 1, height = bottom - pad - (pad + S(40)) - 2;
    textEditorGutter = {pad + 1, top, pad + 1 + gutterWidth, top + height};
    MoveWindow(textEditorBox, textEditorGutter.right, top, area.right - 2 * pad - 2 - gutterWidth, height, TRUE);
    MoveWindow(textEditorCancel, area.right - pad - buttonWidth, bottom, buttonWidth, buttonHeight, TRUE);
    MoveWindow(textEditorOk, area.right - pad - 2 * buttonWidth - S(10), bottom, buttonWidth, buttonHeight, TRUE);
    InvalidateRect(textEditorWindow, NULL, FALSE);
}

int measureLineHeight(HFONT font) {
    HDC screen = GetDC(NULL);
    HGDIOBJ oldFont = gdi.SelectObject(screen, font);
    RECT box = {0, 0, 0, 0};
    DrawTextW(screen, L"0", 1, &box, DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX);
    gdi.SelectObject(screen, oldFont);
    ReleaseDC(NULL, screen);
    return box.bottom - box.top;
}

void drawLineNumbers(HDC canvas) {
    RECT gutter = textEditorGutter;
    int width = gutter.right - gutter.left, height = gutter.bottom - gutter.top;
    if (width <= 0 || height <= 0) return;
    HDC memory = gdi.CreateCompatibleDC(canvas);
    HBITMAP picture = gdi.CreateCompatibleBitmap(canvas, width, height);
    HGDIOBJ oldPicture = gdi.SelectObject(memory, picture);

    RECT box = {0, 0, width, height};
    fill(memory, box, colors.header);
    RECT separator = {width - 1, 0, width, height};
    fill(memory, separator, colors.border);

    int first = askEditor(EM_GETFIRSTVISIBLELINE, 0, 0);
    int count = askEditor(EM_GETLINECOUNT, 0, 0);
    int caret = askEditor(EM_LINEFROMCHAR, (WPARAM)-1, 0);
    RECT text;
    askEditor(EM_GETRECT, 0, (LPARAM)&text);
    RECT client;
    GetClientRect(textEditorBox, &client);
    int bottom = min((int)client.bottom, height);
    int y = text.top;
    for (int line = first; line < count && y < bottom; line++) {
        RECT number = {0, y, width - S(8), min(y + textEditorLineHeight, bottom)};
        drawText(memory, to_string(line + 1), number, fonts.mono, line == caret ? colors.accent : colors.muted,
                 DT_RIGHT | DT_TOP | DT_SINGLELINE);
        y += textEditorLineHeight;
    }

    gdi.BitBlt(canvas, gutter.left, gutter.top, width, height, memory, 0, 0, SRCCOPY);
    gdi.SelectObject(memory, oldPicture);
    gdi.DeleteObject(picture);
    gdi.DeleteDC(memory);
    shownFirstLine = first;
    shownLineCount = count;
    shownCaretLine = caret;
}

void refreshLineNumbers() {
    int first = askEditor(EM_GETFIRSTVISIBLELINE, 0, 0);
    int count = askEditor(EM_GETLINECOUNT, 0, 0);
    int caret = askEditor(EM_LINEFROMCHAR, (WPARAM)-1, 0);
    if (first == shownFirstLine && count == shownLineCount && caret == shownCaretLine) return;
    InvalidateRect(textEditorWindow, &textEditorGutter, FALSE);
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
    border.left = textEditorGutter.left;
    InflateRect(&border, 1, 1);
    frame(canvas, border, colors.border);
    drawLineNumbers(canvas);
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

LRESULT handleTextEditorBox(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
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
    if (message == WM_CHAR && (wParam == 1 || wParam == 10 || wParam == 27)) return 0;
    return CallWindowProcW(textEditorBoxOldProc, hwnd, message, wParam, lParam);
}

LRESULT CALLBACK textEditorBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = handleTextEditorBox(hwnd, message, wParam, lParam);
    if (!askingEditor && message != WM_DESTROY && message != WM_NCDESTROY && hwnd == textEditorBox) refreshLineNumbers();
    return result;
}

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
    textEditorLineHeight = measureLineHeight(fonts.mono);
    shownFirstLine = shownLineCount = shownCaretLine = -1;

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

    MSG message;
    while (!textEditorClosed) {
        int got = GetMessageW(&message, NULL, 0, 0);
        if (got <= 0) {
            PostQuitMessage((int)message.wParam);
            break;
        }
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    TextEditorResult result = {textEditorAccepted, withoutCarriageReturns(windowText(textEditorBox))};

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
