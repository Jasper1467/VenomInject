#include "Utilities.h"

#include <TlHelp32.h>
#include <string>
#include <sstream>
#include <d3d9.h>
#include <iostream>
#include <vector>

HANDLE Utilities::GetProcessByName(std::string szProcessName)
{
    // Process ID
    DWORD pid = 0;

    // Create toolhelp snapshot.
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 process;
    process.dwSize = sizeof(process);

    // Walkthrough all processes.
    if (Process32First(snapshot, &process))
    {
        do
        {
            std::wstring ws(process.szExeFile);
            std::string str(ws.begin(), ws.end());
            if (strcmp(szProcessName.c_str(), str.c_str()) == 0)
            {
                pid = process.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &process));
    }

    CloseHandle(snapshot);

    // Checks if the process exists
    if (pid != 0)
    {
        // Returns the processes handle if it exists
        return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    }

    return NULL;
}

// Helper function to open the file explorer dialog and return the selected path
std::string Utilities::OpenFileExplorerDialog()
{
    char szFilePath[MAX_PATH] = { 0 };

    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr; // Optional: Pass the window handle
    ofn.lpstrFilter = "DLL Files\0*.dll\0All Files\0*.*\0";
    ofn.lpstrFile = szFilePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    ofn.lpstrTitle = "Select a DLL";

    if (GetOpenFileNameA(&ofn))
    {
        return std::string(szFilePath);
    }
    return ""; // Return empty string if dialog is canceled
}

std::string Utilities::GetDllPlatform(const char* szDllPath)
{
    // Open the file
    HANDLE hFile = CreateFileA(szDllPath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return "Unknown";
    }
    // Read the DOS header
    IMAGE_DOS_HEADER dosHeader = { 0 };
    DWORD dwBytesRead = 0;
    ReadFile(hFile, &dosHeader, sizeof(dosHeader), &dwBytesRead, nullptr);
    if (dwBytesRead != sizeof(dosHeader) || dosHeader.e_magic != IMAGE_DOS_SIGNATURE)
    {
        CloseHandle(hFile);
        return "Unknown";
    }
    // Seek to the NT header
    SetFilePointer(hFile, dosHeader.e_lfanew, nullptr, FILE_BEGIN);
    // Read the NT header
    IMAGE_NT_HEADERS ntHeader = { 0 };
    ReadFile(hFile, &ntHeader, sizeof(ntHeader), &dwBytesRead, nullptr);
    if (dwBytesRead != sizeof(ntHeader) || ntHeader.Signature != IMAGE_NT_SIGNATURE)
    {
        CloseHandle(hFile);
        return "Unknown";
    }
    // Determine the platform
    if (ntHeader.FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
    {
        CloseHandle(hFile);
        return "X64";
    }
    else if (ntHeader.FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
    {
        CloseHandle(hFile);
        return "X86";
    }
    CloseHandle(hFile);
    return "Unknown";
}