#include <windows.h>
#include <fstream>
#include <string>
#include <iostream>

// Convert UTF-8 std::string → std::wstring safely
std::wstring toWide(const std::string& input) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, NULL, 0);
    std::wstring output(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, &output[0], size_needed);
    return output;
}

// Convert std::wstring → std::string safely
std::string toNarrow(const std::wstring& input) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, input.c_str(), -1, NULL, 0, NULL, NULL);
    std::string output(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, input.c_str(), -1, &output[0], size_needed, NULL, NULL);
    return output;
}

bool fileExists(const std::string& filePath) {
    std::ifstream file(filePath);
    return file.good();
}

// Get string from INI file (Windows API wrapper)
std::string getIniString(const std::string& iniPath, const std::string& section, const std::string& key, const std::string& defaultValue) {
    wchar_t buffer[4096] = {};
    std::wstring wIniPath = toWide(iniPath);
    std::wstring wSection = toWide(section);
    std::wstring wKey = toWide(key);
    
    DWORD result = GetPrivateProfileStringW(
        wSection.c_str(),
        wKey.c_str(),
        toWide(defaultValue).c_str(),
        buffer,
        sizeof(buffer) / sizeof(wchar_t),
        wIniPath.c_str()
    );
    
    return toNarrow(buffer);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Get the directory where the launcher is located
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);

    std::string currentPath(buffer);
    size_t pos = currentPath.find_last_of("\\/");
    std::string exeDir = (pos != std::string::npos) ? currentPath.substr(0, pos) : ".";

    // INI config file is in the same directory as the launcher
    std::string iniPath = exeDir + "\\union-crax.ini";

    // Check if INI file exists
    if (!fileExists(iniPath)) {
        MessageBoxA(NULL,
            ("Failed to find union-crax.ini config file at:\n" + iniPath).c_str(),
            "Game Launcher Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Read configuration from INI file
    std::string exePath = getIniString(iniPath, "Launcher", "gameExe", "");
    std::string workingDir = getIniString(iniPath, "Launcher", "workingDir", exeDir);
    std::string arguments = getIniString(iniPath, "Launcher", "launchArgs", "");

    // Validate required gameExe
    if (exePath.empty()) {
        MessageBoxA(NULL,
            "The 'gameExe' setting is missing or empty in union-crax.ini.\n"
            "Please configure the gameExe path relative to the launcher.",
            "Game Launcher Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Check if the executable exists
    if (!fileExists(exePath)) {
        MessageBoxA(NULL,
            ("Failed to find the game executable:\n" + exePath).c_str(),
            "Game Launcher Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Build the full command line
    std::string commandLineStr = "\"" + exePath + "\"";
    
    // Add arguments if specified
    if (!arguments.empty()) {
        commandLineStr += " " + arguments;
    }

    std::wstring cmdW = toWide(commandLineStr);
    std::wstring workingDirW = toWide(workingDir);

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    // Launch the game
    BOOL result = CreateProcessW(
        NULL,
        &cmdW[0],             // writable buffer required by WinAPI
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        workingDirW.empty() ? NULL : workingDirW.c_str(),
        &si,
        &pi
    );

    if (!result) {
        DWORD error = GetLastError();
        MessageBoxA(NULL,
            ("Failed to launch the game.\n"
             "Error code: " + std::to_string(error) + "\n\n"
             "Command: " + commandLineStr + "\n"
             "Working directory: " + workingDir).c_str(),
            "Game Launcher Error",
            MB_OK | MB_ICONERROR);
        return 1;
    }

    // Clean up handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}
