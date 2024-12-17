#pragma once

#include <Windows.h>

namespace Injector
{
	namespace ManualMapping
	{
		void Inject(HANDLE hProcess, const char* szDllPath);
	}
}