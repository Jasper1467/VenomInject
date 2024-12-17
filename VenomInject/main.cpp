#include "Menu/Menu.h"

#include <thread>

#include <CompileTimeRandom.hpp>

int __stdcall wWinMain(
	HINSTANCE instance,
	HINSTANCE previousInstance,
	PWSTR arguments,
	int commandShow)
{
	constexpr CCompileTimeRandom rng;
	auto szRandomWindowName = rng.GenerateRandomString<10>();

	Menu::CreateHWindow(szRandomWindowName.data());
	Menu::CreateDevice();
	Menu::CreateImGui();

	while (Menu::g_bIsRunning)
	{
		Menu::BeginRender();
		Menu::Render();
		Menu::EndRender();

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	Menu::DestroyImGui();
	Menu::DestroyDevice();
	Menu::DestroyHWindow();

	return EXIT_SUCCESS;
}