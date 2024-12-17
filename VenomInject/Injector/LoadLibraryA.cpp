#include "LoadLibraryA.h"

#include <stdexcept>

void Injector::LoadLibraryA::Inject(HANDLE hProcess, const char* szDllPath)
{
    // Attempt to allocate memory in the target process
    void* pAddress = VirtualAllocEx(hProcess, nullptr, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pAddress)
    {
        const DWORD error = GetLastError();
        throw std::runtime_error("Failed to allocate memory in target process");
    }

    // Attempt to write the DLL path into the allocated memory
    if (!WriteProcessMemory(hProcess, pAddress, szDllPath, strlen(szDllPath) + 1, nullptr))
    {
        const DWORD error = GetLastError();
        VirtualFreeEx(hProcess, pAddress, 0, MEM_RELEASE); // Cleanup
        throw std::runtime_error("Failed to write to target process memory");
    }

    // Attempt to create a remote thread to load the DLL
    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)::LoadLibraryA, pAddress, 0, nullptr);
    if (!hThread)
    {
        const DWORD error = GetLastError();
        VirtualFreeEx(hProcess, pAddress, 0, MEM_RELEASE);  // Cleanup
        throw std::runtime_error("Failed to create remote thread in target process");
    }

    // If all steps succeed, close the handle to the thread
    CloseHandle(hThread);
}