#pragma once

#include <Windows.h>
#include <string>
#include <imgui.h>

namespace Utilities
{
	HANDLE GetProcessByName(std::string szProcessName);
	std::string OpenFileExplorerDialog();

	std::string GetDllPlatform(const char* szDllPath);
}