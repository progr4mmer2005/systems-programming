// Функции Windows, которые Code Runner сам не подключает.
//
// Code Runner собирает командой "g++ labN.cpp -o labN" без дополнительных флагов.
// Без флагов не подключаются библиотеки рисования (gdi32) и таблиц (comctl32).
// Поэтому нужные функции загружаются из DLL при запуске.
//
// После loadSystemFunctions() функции вызываются через структуру: gdi.CreateSolidBrush(...).
#pragma once

#include "winapi.h"

struct GdiFunctions {
    decltype(&CreateFontW) CreateFontW;
    decltype(&CreateSolidBrush) CreateSolidBrush;
    decltype(&CreatePen) CreatePen;
    decltype(&SelectObject) SelectObject;
    decltype(&DeleteObject) DeleteObject;
    decltype(&SetTextColor) SetTextColor;
    decltype(&SetBkColor) SetBkColor;
    decltype(&SetBkMode) SetBkMode;
    decltype(&RoundRect) RoundRect;
    decltype(&Ellipse) Ellipse;
    decltype(&GetDeviceCaps) GetDeviceCaps;
    decltype(&CreateCompatibleDC) CreateCompatibleDC;
    decltype(&CreateCompatibleBitmap) CreateCompatibleBitmap;
    decltype(&BitBlt) BitBlt;
    decltype(&DeleteDC) DeleteDC;
};

struct ComctlFunctions {
    decltype(&InitCommonControlsEx) InitCommonControlsEx;
    decltype(&ImageList_Create) ImageList_Create;
};

// Необязательные: тёмные полосы прокрутки и тёмный заголовок окна.
// Если их нет (старая Windows), программа просто работает без этого.
struct ThemeFunctions {
    HRESULT(WINAPI* SetWindowTheme)(HWND, LPCWSTR, LPCWSTR);
    HRESULT(WINAPI* DwmSetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD);
};

GdiFunctions gdi = {};
ComctlFunctions comctl = {};
ThemeFunctions themeApi = {};

bool loadSystemFunctions() {
    HMODULE dll = LoadLibraryW(L"gdi32.dll");
    gdi.CreateFontW = (decltype(gdi.CreateFontW))GetProcAddress(dll, "CreateFontW");
    gdi.CreateSolidBrush = (decltype(gdi.CreateSolidBrush))GetProcAddress(dll, "CreateSolidBrush");
    gdi.CreatePen = (decltype(gdi.CreatePen))GetProcAddress(dll, "CreatePen");
    gdi.SelectObject = (decltype(gdi.SelectObject))GetProcAddress(dll, "SelectObject");
    gdi.DeleteObject = (decltype(gdi.DeleteObject))GetProcAddress(dll, "DeleteObject");
    gdi.SetTextColor = (decltype(gdi.SetTextColor))GetProcAddress(dll, "SetTextColor");
    gdi.SetBkColor = (decltype(gdi.SetBkColor))GetProcAddress(dll, "SetBkColor");
    gdi.SetBkMode = (decltype(gdi.SetBkMode))GetProcAddress(dll, "SetBkMode");
    gdi.RoundRect = (decltype(gdi.RoundRect))GetProcAddress(dll, "RoundRect");
    gdi.Ellipse = (decltype(gdi.Ellipse))GetProcAddress(dll, "Ellipse");
    gdi.GetDeviceCaps = (decltype(gdi.GetDeviceCaps))GetProcAddress(dll, "GetDeviceCaps");
    gdi.CreateCompatibleDC = (decltype(gdi.CreateCompatibleDC))GetProcAddress(dll, "CreateCompatibleDC");
    gdi.CreateCompatibleBitmap = (decltype(gdi.CreateCompatibleBitmap))GetProcAddress(dll, "CreateCompatibleBitmap");
    gdi.BitBlt = (decltype(gdi.BitBlt))GetProcAddress(dll, "BitBlt");
    gdi.DeleteDC = (decltype(gdi.DeleteDC))GetProcAddress(dll, "DeleteDC");

    dll = LoadLibraryW(L"comctl32.dll");
    comctl.InitCommonControlsEx = (decltype(comctl.InitCommonControlsEx))GetProcAddress(dll, "InitCommonControlsEx");
    comctl.ImageList_Create = (decltype(comctl.ImageList_Create))GetProcAddress(dll, "ImageList_Create");

    dll = LoadLibraryW(L"uxtheme.dll");
    if (dll) themeApi.SetWindowTheme = (decltype(themeApi.SetWindowTheme))GetProcAddress(dll, "SetWindowTheme");
    dll = LoadLibraryW(L"dwmapi.dll");
    if (dll) themeApi.DwmSetWindowAttribute = (decltype(themeApi.DwmSetWindowAttribute))GetProcAddress(dll, "DwmSetWindowAttribute");

    if (!gdi.CreateFontW || !gdi.BitBlt || !comctl.InitCommonControlsEx || !comctl.ImageList_Create) return false;

    // Включить таблицы и остальные стандартные элементы.
    INITCOMMONCONTROLSEX controls = {sizeof(controls), ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES};
    comctl.InitCommonControlsEx(&controls);
    return true;
}
