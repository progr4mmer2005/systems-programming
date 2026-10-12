#pragma once

#define UNICODE
#define _UNICODE
#define NOMINMAX
#define _WIN32_WINNT 0x0601
#define _WIN32_IE 0x0700
#include <windows.h>
#include <commctrl.h>

#ifndef LVS_EX_DOUBLEBUFFER
#define LVS_EX_DOUBLEBUFFER 0x00010000
#endif
