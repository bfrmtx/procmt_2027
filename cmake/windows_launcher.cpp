#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdio>
#include <string>
#include <vector>

namespace {
int fail(const wchar_t *message) {
    const DWORD error = GetLastError();
    wchar_t *detail = nullptr;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_IGNORE_INSERTS,
                   nullptr, error, 0, reinterpret_cast<wchar_t *>(&detail), 0, nullptr);
    std::wstring text = message;
    if (detail) {
        text += L"\n";
        text += detail;
        LocalFree(detail);
    }
#ifdef PROCMT_GUI_LAUNCHER
    MessageBoxW(nullptr, text.c_str(), L"ProcMT launcher", MB_OK | MB_ICONERROR);
#else
    std::fwprintf(stderr, L"%ls\n", text.c_str());
#endif
    return 1;
}

int launch() {
    std::vector<wchar_t> filename(32768);
    DWORD length = GetModuleFileNameW(nullptr, filename.data(), static_cast<DWORD>(filename.size()));
    if (!length || length >= filename.size())
        return fail(L"Cannot determine the launcher path.");

    std::wstring path(filename.data(), length);
    const auto slash = path.find_last_of(L"\\/");
    if (slash == std::wstring::npos) {
        SetLastError(ERROR_BAD_PATHNAME);
        return fail(L"Cannot determine the installation directory.");
    }
    const std::wstring bin = path.substr(0, slash);
    const std::wstring name = path.substr(slash + 1);
    const std::wstring executable = bin + L"\\..\\libexec\\" + name;
    const std::wstring library = bin + L"\\..\\lib";

    DWORD size = GetEnvironmentVariableW(L"PATH", nullptr, 0);
    std::vector<wchar_t> old_path(size ? size : 1);
    if (size && !GetEnvironmentVariableW(L"PATH", old_path.data(), size))
        return fail(L"Cannot read PATH.");
    std::wstring new_path = library + L";" + old_path.data();
    if (!SetEnvironmentVariableW(L"PATH", new_path.c_str()))
        return fail(L"Cannot set PATH.");

    const wchar_t *arguments = GetCommandLineW();
    if (*arguments == L'"') {
        ++arguments;
        while (*arguments && *arguments != L'"')
            ++arguments;
        if (*arguments)
            ++arguments;
    } else {
        while (*arguments && *arguments != L' ' && *arguments != L'\t')
            ++arguments;
    }
    std::wstring command = L"\"" + executable + L"\"" + arguments;
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(executable.c_str(), command.data(), nullptr, nullptr, TRUE, 0,
                        nullptr, nullptr, &startup, &process))
        return fail(L"Cannot start the ProcMT executable.");

    CloseHandle(process.hThread);
    if (WaitForSingleObject(process.hProcess, INFINITE) != WAIT_OBJECT_0) {
        const DWORD error = GetLastError();
        CloseHandle(process.hProcess);
        SetLastError(error);
        return fail(L"Cannot wait for the ProcMT executable.");
    }
    DWORD exit_code = 1;
    if (!GetExitCodeProcess(process.hProcess, &exit_code)) {
        const DWORD error = GetLastError();
        CloseHandle(process.hProcess);
        SetLastError(error);
        return fail(L"Cannot obtain the ProcMT exit code.");
    }
    CloseHandle(process.hProcess);
    return static_cast<int>(exit_code);
}
}

#ifdef PROCMT_GUI_LAUNCHER
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    return launch();
}
#else
int wmain() {
    return launch();
}
#endif
