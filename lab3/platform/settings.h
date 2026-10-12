// Настройки, которые запоминаются между запусками. Хранятся в реестре Windows
// в разделе HKEY_CURRENT_USER\Software\SystemsProgrammingLabs.
#pragma once

#include "winapi.h"

const wchar_t* SETTINGS_KEY = L"Software\\SystemsProgrammingLabs";

// Прочитать число из реестра. Нет такого значения - вернуть fallback.
DWORD readNumber(const wchar_t* keyName, const wchar_t* valueName, DWORD fallback) {
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, keyName, 0, KEY_READ, &key) != ERROR_SUCCESS) return fallback;
    DWORD value = 0, size = sizeof(value), type = 0;
    LONG status = RegQueryValueExW(key, valueName, NULL, &type, (BYTE*)&value, &size);
    RegCloseKey(key);
    if (status != ERROR_SUCCESS || type != REG_DWORD) return fallback;
    return value;
}

void writeNumber(const wchar_t* valueName, DWORD value) {
    HKEY key;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, SETTINGS_KEY, 0, NULL, 0, KEY_WRITE, NULL, &key, NULL) != ERROR_SUCCESS) return;
    RegSetValueExW(key, valueName, 0, REG_DWORD, (BYTE*)&value, sizeof(value));
    RegCloseKey(key);
}

// Тема окна: как в Windows, светлая или тёмная.
const int THEME_SYSTEM = 0;
const int THEME_LIGHT = 1;
const int THEME_DARK = 2;

int loadThemeMode() {
    int mode = (int)readNumber(SETTINGS_KEY, L"ThemeMode", THEME_SYSTEM);
    return mode >= THEME_SYSTEM && mode <= THEME_DARK ? mode : THEME_SYSTEM;
}

void saveThemeMode(int mode) { writeNumber(L"ThemeMode", (DWORD)mode); }

// Включена ли в Windows тёмная тема для приложений (Параметры → Персонализация → Цвета).
bool systemUsesDarkTheme() {
    return readNumber(L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"AppsUseLightTheme", 1) == 0;
}

bool isDarkMode(int mode) { return mode == THEME_DARK || (mode == THEME_SYSTEM && systemUsesDarkTheme()); }
