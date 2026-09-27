// Настройки, которые запоминаются между запусками. Хранятся в реестре Windows
// в разделе HKEY_CURRENT_USER\Software\SystemsProgrammingLabs.
#pragma once

#include "winapi.h"

const wchar_t* SETTINGS_KEY = L"Software\\SystemsProgrammingLabs";

bool loadDarkTheme() {
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, SETTINGS_KEY, 0, KEY_READ, &key) != ERROR_SUCCESS) return false;
    DWORD value = 0, size = sizeof(value), type = 0;
    LONG status = RegQueryValueExW(key, L"DarkTheme", NULL, &type, (BYTE*)&value, &size);
    RegCloseKey(key);
    return status == ERROR_SUCCESS && type == REG_DWORD && value != 0;
}

void saveDarkTheme(bool dark) {
    HKEY key;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, SETTINGS_KEY, 0, NULL, 0, KEY_WRITE, NULL, &key, NULL) != ERROR_SUCCESS) return;
    DWORD value = dark ? 1 : 0;
    RegSetValueExW(key, L"DarkTheme", 0, REG_DWORD, (BYTE*)&value, sizeof(value));
    RegCloseKey(key);
}
