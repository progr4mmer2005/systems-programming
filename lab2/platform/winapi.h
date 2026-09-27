// Подключение Windows. Этот файл подключается первым.
#pragma once

#define UNICODE              // Windows-функции работают с текстом в UTF-16
#define _UNICODE
#define NOMINMAX             // чтобы windows.h не объявлял свои min и max
#define _WIN32_WINNT 0x0601  // Windows 7 и новее
#define _WIN32_IE 0x0700
#include <windows.h>
#include <commctrl.h>

// В заголовках старого MinGW (GCC 6.3) этой константы нет.
#ifndef LVS_EX_DOUBLEBUFFER
#define LVS_EX_DOUBLEBUFFER 0x00010000
#endif
