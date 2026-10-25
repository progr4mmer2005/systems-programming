#pragma once

#include "../config.h"
#include "../platform/startup.h"
#include "actions.h"
#include "controls.h"
#include "events.h"
#include "layout.h"
#include "paint.h"

void onCommand(int id, int event) {
    if (event == BN_CLICKED && id == ID_PASS1) runPass1();
    if (event == BN_CLICKED && id == ID_PASS2) runPass2();
    if (event == BN_CLICKED && id == ID_RESET) {
        resetResults();
        setStatus("Результаты очищены. Нажмите «Первый проход» (F5).");
    }
    if (event == CBN_SELCHANGE && id == ID_EXAMPLES) {
        int index = (int)SendMessageW(examplesList, CB_GETCURSEL, 0, 0);
        loadExample(index);
    }
    if (event == EN_CHANGE && id == ID_ADDRESS) onAddressEdited();
}

LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_CREATE) {
        createControls(window);
        return 0;
    }
    if (message == WM_SIZE) {
        if (mainWindow) layout();
        return 0;
    }
    if (message == WM_GETMINMAXINFO) {
        MINMAXINFO* limits = (MINMAXINFO*)lParam;
        limits->ptMinTrackSize.x = S(1180);
        limits->ptMinTrackSize.y = S(720);
        return 0;
    }
    if (message == WM_ERASEBKGND) return 1;
    if (message == WM_PAINT) {
        paintWithoutFlicker(window);
        return 0;
    }
    if (message == WM_NOTIFY) {
        NMHDR* note = (NMHDR*)lParam;
        for (Grid* grid : allGrids) {
            LRESULT result = 0;
            if (note->hwndFrom == grid->hwnd && grid->handleNotify(note, result)) return result;
        }
    }
    if (message == WM_COMMAND) {
        onCommand(LOWORD(wParam), HIWORD(wParam));
        return 0;
    }
    if (message == WM_SYSCOLORCHANGE) {
        loadSystemColors();
        for (Grid* grid : allGrids) grid->applyTheme();
        RedrawWindow(window, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
    }
    if (message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

bool handleHotkey(MSG message) {
    if (message.message != WM_KEYDOWN) return false;
    if (message.wParam != VK_F5 && message.wParam != VK_F6) return false;
    HWND focus = GetFocus();
    if (focus) SendMessageW(focus, WM_KILLFOCUS, 0, 0);
    if (message.wParam == VK_F5) runPass1();
    if (message.wParam == VK_F6 && IsWindowEnabled(pass2Button)) runPass2();
    return true;
}

int runApp() {
    enableSharpText();
    enableModernControls();
    if (!loadSystemFunctions()) {
        MessageBoxW(NULL, L"Не удалось загрузить gdi32.dll / comctl32.dll", L"Ошибка", MB_ICONERROR);
        return 1;
    }
    HDC screen = GetDC(NULL);
    screenDpi = gdi.GetDeviceCaps(screen, LOGPIXELSY);
    ReleaseDC(NULL, screen);
    createFonts();
    loadSystemColors();

    WNDCLASSEXW windowClass = {};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = GetModuleHandleW(NULL);
    windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
    windowClass.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    windowClass.lpszClassName = L"AsmLab1Window";
    RegisterClassExW(&windowClass);

    string title = LAB_TITLE;
    HWND window = CreateWindowExW(0, L"AsmLab1Window", toWide(title).c_str(), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT,
                                  CW_USEDEFAULT, S(1440), S(880), NULL, NULL, GetModuleHandleW(NULL), NULL);
    ShowWindow(window, SW_SHOWMAXIMIZED);
    UpdateWindow(window);

    MSG message;
    while (GetMessageW(&message, NULL, 0, 0) > 0) {
        if (handleHotkey(message)) continue;
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return 0;
}
