# UnionCrax.Launcher

A small Windows launcher that reads `union-crax.ini` and starts the configured game executable via `CreateProcessW`.

---

## Requirements

| Tool | Where to get it |
|------|----------------|
| Visual Studio Build Tools 2019 or 2022 | https://aka.ms/vs/17/release/vs_BuildTools.exe |
| Workload: **Desktop development with C++** (or the *C++ build tools* individual component) | VS Installer |
| An `icon.ico` file in the project root (optional – remove the `IDI_ICON1` line from `uc.launcher.rc` if you don't have one) | — |

---

## Building with `cl.exe` (VS Build Tools) – recommended quick path

The included `build.bat` script automatically locates your VS / Build Tools installation via `vswhere.exe`, initialises the x64 environment, and builds the executable in a single step.

```bat
:: Release build (default)
build.bat

:: Debug build
build.bat Debug
```

Output: `uc.launcher.exe` in the project root.

### What the script does

1. Uses `vswhere.exe` to find the latest MSVC toolset.
2. Calls `vcvarsall.bat x64` to set up the compiler environment.
3. Runs `rc.exe` to compile `uc.launcher.rc` → `uc.launcher.res`.
4. Runs `cl.exe /std:c++17 …` to compile and link everything into `uc.launcher.exe`.
5. Deletes intermediate files (`.res`, `.obj`).

### Manual `cl.exe` invocation

If you prefer to run the commands yourself (e.g. inside a *Developer Command Prompt for VS* or after running `vcvarsall.bat x64`):

```bat
rc.exe /nologo /fo uc.launcher.res uc.launcher.rc

cl.exe /nologo /std:c++17 /EHsc /W3 /O2 /DNDEBUG ^
    uc.launcher.cpp uc.launcher.res ^
    /Fe:uc.launcher.exe ^
    /link /SUBSYSTEM:WINDOWS /NOLOGO ^
    kernel32.lib user32.lib
```

---

## Building with CMake

CMake 3.18+ and Ninja are required (both ship with VS / Build Tools).

```bat
:: Open a "Developer Command Prompt for VS" (or run vcvarsall.bat x64 first)

cmake -B out\build\x64-Release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build out\build\x64-Release
```

Output: `out\build\x64-Release\bin\uc.launcher.exe`.

Visual Studio and VS Code (with the CMake Tools extension) can also open the project directly using `CMakeSettings.json`.

---

## Configuration (`union-crax.ini`)

Place `union-crax.ini` in the same directory as the built `uc.launcher.exe`:

```ini
[Launcher]
; Path to the game executable (relative or absolute)
gameExe=Game\Binaries\Win64\MyGame.exe

; Working directory for the game process (defaults to launcher directory)
workingDir=Game\Binaries\Win64

; Optional command-line arguments passed to the game
launchArgs=-dx12
```
