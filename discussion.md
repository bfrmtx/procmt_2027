# dll patch

The Clean MSVC + CMake Solution (No Code Changes)

MSVC supports an advanced delay-load notification hook. When CMake adds the /DELAYLOAD flag, you can tell the compiler to run a tiny snippet of code automatically the moment Windows tries to load your DLL, completely behind the scenes.
You can package this helper into a single .cpp file (e.g., windows_rpath.cpp), and add it to your CMake targets only when compiling on Windows.

1. Create windows_rpath.cpp (Keep it in your project root)

This file intercepts the MSVC delay-load helper and forces Windows to resolve the path dynamically right before loading the DLL.

```cpp
#ifdef \_WIN32
#include <windows.h>
#include <delayhlp.h>
#include <filesystem>

// This hook triggers automatically right before a delay-loaded DLL is loaded
FARPROC WINAPI delay_load_hook(unsigned dliNotify, PDelayLoadInfo pdli) {
if (dliNotify == dliNotePreLoadLibrary) {
wchar_t buffer\[MAX_PATH\];
if (GetModuleFileNameW(NULL, buffer, MAX_PATH) != 0) {
std::filesystem::path bin_dir = std::filesystem::path(buffer).parent_path();
std::filesystem::path lib_dir = bin_dir.parent_path() / "lib";

        // Set the search directory just-in-time
        SetDllDirectoryW(lib_dir.c_str());
    }
}
return NULL;

}

// Register our hook into the MSVC delay-load runtime system
ExternC const PfnDliHook \__pfnDliNotifyHook2 = delay_load_hook;
#endif
```
  


2. Update your CMakeLists.txt

Now, update your build file. When CMake detects a Windows/MSVC target, it attaches this single source file and applies the delay-load flags. You don't need to touch your application main() files at all.

cmake_minimum_required(VERSION 3.31)
project(MyProject CXX)

# Standard source files for your application (Clean C++, no Windows headers)

set(APP_SOURCES main.cpp)

# On Windows, conditionally append our RPATH emulation file
```cpp

if(WIN32 AND MSVC)
list(APPEND APP_SOURCES windows_rpath.cpp)
endif()

add_executable(my_app ${APP_SOURCES})

# --- Linux Config (Standard) ---

if(UNIX)
set_target_properties(my_app PROPERTIES CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()

# --- Windows Native MSVC Config ---

if(WIN32 AND MSVC)
\# Tell MSVC to delay-load the specific third-party library
target_link_options(my_app PRIVATE "/DELAYLOAD:libfftw3-3.dll")

```

# Required Windows system library to handle delay-loading mechanics
```cpp
target_link_libraries(my_app PRIVATE delayimp)
endif()
```

this is perfect for your workflow

• Pure Linux codebase stays untouched: Your source code files (like main.cpp) remain 100% platform-agnostic, standard C++. You won't see #ifdef \_WIN32 or Windows API functions mixed into your core code.
• Maintains layout parity: Your build artifacts will deploy flawlessly into mysw/bin on both Linux and Windows, drawing shared components natively from mysw/lib without file duplication.
• Scalable: If you add more mains or tests later, you simply add windows_rpath.cpp to their CMake source lists under the if(WIN32) block, and they automatically inherit the feature.

—————-

When dealing with Qt6 (and large frameworks like QCustomPlot) alongside multiple internal binaries, /DELAYLOAD starts to fall apart.
Frameworks like Qt6 do not just load basic DLLs; they use an internal runtime plugin architecture (e.g., loading qwindows.dll from a plugins/platforms directory). If you try to delay-load Qt6, your application will face continuous, hard-to-predict runtime crashes because it is impossible to hardcode /DELAYLOAD switches for dozens of hidden, nested framework sub-DLLs.
Since you have a native MSVC build environment on Windows, you can implement a seamless, automated setup. You can use a Global Initialization Injection technique via CMake, allowing you to achieve the exact same structure as Linux rpath without changing a single line of your C++ code.

The Strategy: Automated Proxy Initializer

Instead of adding headers or delay-loading every library, create a single, platform-specific proxy source file. Using CMake, you will instruct MSVC to inject this file into every executable automatically at compile time.
The proxy file uses a compiler initialization trick to invoke SetDllDirectoryW before your application's main() function even runs, bypassing the need for /DELAYLOAD entirely.

1. Create windows_rpath.cpp (Keep it in your project root)

This script calculates the directory structure dynamically (climbing out of bin and entering lib).
```cpp
#ifdef \_WIN32
#include <windows.h>
#include <filesystem>

// This function runs automatically during the static initialization phase,
// BEFORE the OS loader evaluates the main application's linked DLLs.
inline struct WindowsRPathSimulator {
WindowsRPathSimulator() {
wchar_t buffer\[MAX_PATH\];
if (GetModuleFileNameW(NULL, buffer, MAX_PATH) != 0) {
std::filesystem::path bin_dir = std::filesystem::path(buffer).parent_path();
std::filesystem::path lib_dir = bin_dir.parent_path() / "lib";
        // Re-route the Windows global DLL loader path seamlessly to mysw/lib
        SetDllDirectoryW(lib_dir.c_str());
    }
}

} g_windows_rpath_simulator;
#endif
```
Use code with caution.

2. Configure a Global Macro in your root CMakeLists.txt

To apply this across all your binaries, Qt apps, and tests without manually editing every single add_executable block, you can override or extend the standard CMake add_executable command using a macro.
Add this logic to your top-level CMakeLists.txt:
cmake
cmake_minimum_required(VERSION 3.31)
project(MyMultiBinaryProject CXX)

# --- Define the Cross-Platform RPATH Emulator ---
```cpp

macro(add_my_executable TARGET_NAME)
\# Collect all additional source files passed to the macro
set(TARGET_SRCS ${ARGN})

# If on Windows using MSVC, automatically inject our hidden initializer file
if(WIN32 AND MSVC)
    list(APPEND TARGET_SRCS "${CMAKE_SOURCE_DIR}/windows_rpath.cpp")
endif()

# Pass the compiled list down to standard CMake
add_executable(${TARGET_NAME} ${TARGET_SRCS})

# Keep your clean Linux RPATH intact
if(UNIX)
    set_target_properties(${TARGET_NAME} PROPERTIES CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()

endmacro()
```
Use code with caution.

3. Update your sub-projects or target files

Now, when you define any binary (including your complex Qt6 or QCustomPlot interfaces), swap add_executable out for your new global macro:
cmake

# Instead of add_executable(my_qt_app main.cpp window.cpp)

add_my_executable(my_qt_app main.cpp window.cpp)

# Link your Qt6 components normally

target_link_libraries(my_qt_app PRIVATE Qt6::Widgets Qt6::Core QCustomPlot)
Use code with caution.

What happens behind the scenes on Windows?

1. MSVC compiles windows_rpath.cpp directly into your target binary (mysw/bin/my_qt_app.exe).
2. The moment a user double-clicks the executable, the global object g_windows_rpath_simulator instantiates first.
3. Windows updates the lookup paths to include mysw/lib.
4. Qt6, QCustomPlot, FFTW, and any other sub-dependencies are resolved out of the lib folder.
5. Your application transitions into main() with its original code completely untouched.

Handling Qt6 Plugins (Bonus Configuration)

Because Qt6 loads its interface modules (like qwindows.dll) from a separate plugins folder, ensure your installation deployment directory mimics the structure below:
text
procmt/
├── bin/
│   └── my_qt_app.exe
└── lib/
    ├── Qt6Core.dll
    ├── Qt6Widgets.dll
    ├── QCustomPlot.dll
└── platforms/         <-- Copy Qt's plugin folder here
└── qwindows.dll
Use code with caution.
Qt6 naturally knows how to look for its own sub-plugins relative to where its primary Qt6Core.dll is located, allowing you to maintain a clean workspace on both operating systems.
Are your libraries (like QCustomPlot) being built as shared libraries (.dll) or static libraries (.lib) within your project? If you have internal shared libraries that depend on each other, let me know so we can ensure their properties match!