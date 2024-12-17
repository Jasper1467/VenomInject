#pragma once

#include <Windows.h>

namespace Injector
{
	namespace LoadLibraryA
	{
		void Inject(HANDLE hProcess, const char* szDllPath);
	}
}