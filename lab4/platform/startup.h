#pragma once

#include <string>

using namespace std;

#include "winapi.h"

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

void enableSharpText() {
    typedef BOOL(WINAPI * Function)();
    Function setDpiAware = (Function)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetProcessDPIAware");
    if (setDpiAware) setDpiAware();
}

void hideConsoleWindow() {
    DWORD processes[4];
    if (GetConsoleWindow() && GetConsoleProcessList(processes, 4) == 1) FreeConsole();
}
