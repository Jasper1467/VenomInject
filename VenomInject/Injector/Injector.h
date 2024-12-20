#pragma once

#include <Windows.h>
#include <vector>
#include <string>

namespace Injector
{
	enum Method_e
	{
		METHOD_LOADLIBRARYA = 0,
		METHOD_MANUALMAPPING
	};

	inline const char* g_szMethodNames[] =
	{
		"LoadLibraryA",
		"Manual Mapping"
	};

	// Helper to get the number of available methods
	inline size_t GetMethodCount()
	{
		return sizeof(g_szMethodNames) / sizeof(g_szMethodNames[0]);
	}

	void InjectInternal(HANDLE hProcess, const char* szDllPath, Method_e nInjectionMethod);
	void Inject(HANDLE hProcess, std::vector<std::string> szDllPaths, Method_e nInjectionMethod);
}