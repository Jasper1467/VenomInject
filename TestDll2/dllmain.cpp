#include <Windows.h>

DWORD WINAPI MainThread(LPVOID lpParam)
{
	MessageBoxA(0, "Hello from TestDll2!", "TestDll2", MB_OK);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	DisableThreadLibraryCalls(hModule);

	if (dwReason == DLL_PROCESS_ATTACH)
		CreateThread(0, 0, MainThread, 0, 0, 0);

	return TRUE;
}