#pragma once

#include "ManualMapping.h"
#include <fstream>

// Function to load a DLL file into memory
BYTE* LoadFileToMemory(const char* szFilePath, SIZE_T& nFileSize)
{
    std::ifstream File(szFilePath, std::ios::binary | std::ios::ate);
    if (!File.is_open())
        return nullptr;

    nFileSize = File.tellg();
    File.seekg(0, std::ios::beg);

    BYTE* pBuffer = new BYTE[nFileSize];
    File.read(reinterpret_cast<char*>(pBuffer), nFileSize);
    File.close();

    return pBuffer;
}

void Injector::ManualMapping::Inject(HANDLE hProcess, const char* szDllPath)
{
	SIZE_T nDllFileSize = 0;
	BYTE* pDllBuffer = LoadFileToMemory(szDllPath, nDllFileSize);

	if (!pDllBuffer)
	{
		throw std::runtime_error("Failed to load the DLL file!");
		return;
	}

    // Check if the buffer contains a valid PE file
    auto dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(pDllBuffer);
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        throw std::runtime_error("Invalid DOS header\n");
        return;
    }

    auto ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(pDllBuffer + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        throw std::runtime_error("Invalid NT headers\n");
        return;
    }

    // Allocate memory in the target process
    SIZE_T imageSize = ntHeaders->OptionalHeader.SizeOfImage;
    LPVOID remoteImage = VirtualAllocEx(hProcess, nullptr, imageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remoteImage) {
        throw std::runtime_error("Failed to allocate memory in target process\n");
        return;
    }

    // Copy the headers to the target process
    if (!WriteProcessMemory(hProcess, remoteImage, pDllBuffer, ntHeaders->OptionalHeader.SizeOfHeaders, nullptr)) {
        throw std::runtime_error("Failed to write headers to target process\n");
        VirtualFreeEx(hProcess, remoteImage, 0, MEM_RELEASE);
        return;
    }

    // Copy each section to the target process
    auto sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);
    for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
        auto section = &sectionHeader[i];
        if (!WriteProcessMemory(hProcess,
            reinterpret_cast<LPVOID>(reinterpret_cast<BYTE*>(remoteImage) + section->VirtualAddress),
            pDllBuffer + section->PointerToRawData,
            section->SizeOfRawData,
            nullptr))
        {
        
            VirtualFreeEx(hProcess, remoteImage, 0, MEM_RELEASE);
            throw std::runtime_error("Failed to write section to target process\n");
            return;
        }
    }

    // Resolve imports and relocations (not implemented in full here for brevity)
    // Add code to process IMAGE_IMPORT_DESCRIPTOR and IMAGE_BASE_RELOCATION if necessary

    // Execute the DLL's entry point
    LPVOID entryPoint = reinterpret_cast<LPVOID>(reinterpret_cast<BYTE*>(remoteImage) + ntHeaders->OptionalHeader.AddressOfEntryPoint);
    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(entryPoint), nullptr, 0, nullptr);

    if (!hThread) {
        VirtualFreeEx(hProcess, remoteImage, 0, MEM_RELEASE);
        throw std::runtime_error("Failed to create remote thread\n");
        return;
    }

    // Wait for the thread to finish
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
}