// Подготовка при запуске: современный вид окон, чёткость на экранах
// с масштабом, скрытие лишнего консольного окна.
#pragma once

#include <string>

using namespace std;

#include "winapi.h"

// Новый вид таблиц и кнопок (Common Controls 6.0) включается манифестом.
// Обычно его добавляют в exe через ресурсы. Чтобы собирать одной командой,
// манифест пишется во временный файл и подключается при запуске.
void enableModernControls() {
    wchar_t folder[MAX_PATH];
    GetTempPathW(MAX_PATH, folder);
    wstring path = wstring(folder) + L"asm_lab_comctl6.manifest";
    string manifest =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
        "<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">"
        "<dependency><dependentAssembly><assemblyIdentity type=\"win32\" "
        "name=\"Microsoft.Windows.Common-Controls\" version=\"6.0.0.0\" processorArchitecture=\"*\" "
        "publicKeyToken=\"6595b64144ccf1df\" language=\"*\"/></dependentAssembly></dependency></assembly>";
    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return;
    DWORD written = 0;
    WriteFile(file, manifest.c_str(), (DWORD)manifest.size(), &written, NULL);
    CloseHandle(file);

    ACTCTXW context = {};
    context.cbSize = sizeof(context);
    context.lpSource = path.c_str();
    HANDLE handle = CreateActCtxW(&context);
    if (handle == INVALID_HANDLE_VALUE) return;
    ULONG_PTR cookie = 0;
    ActivateActCtx(handle, &cookie);
}

// Без этого при масштабе 125-150% Windows растягивает окно и текст размыт.
// SetProcessDPIAware нет в заголовках старого MinGW, поэтому берётся из user32.dll.
void enableSharpText() {
    typedef BOOL(WINAPI * Function)();
    Function setDpiAware = (Function)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetProcessDPIAware");
    if (setDpiAware) setDpiAware();
}

// При запуске двойным кликом открывается консольное окно, его нужно скрыть.
// При запуске из терминала консоль общая, её не трогаем.
void hideConsoleWindow() {
    DWORD processes[4];
    if (GetConsoleWindow() && GetConsoleProcessList(processes, 4) == 1) FreeConsole();
}
