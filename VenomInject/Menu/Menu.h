#pragma once
#include <d3d9.h>

namespace Menu
{
	// constant window size
	constexpr int WIDTH = 1100;
	constexpr int HEIGHT = 600;

	// when this changes, exit threads
	// and close menu :)
	inline bool g_bIsRunning = true;

	// winapi window vars
	inline HWND g_hWnd = nullptr;
	inline WNDCLASSEX g_hWndClass = { };

	// points for window movement
	inline POINTS g_Position = { };

	// direct x state vars
	inline PDIRECT3D9 g_pD3d = nullptr;
	inline LPDIRECT3DDEVICE9 g_pDevice = nullptr;
	inline D3DPRESENT_PARAMETERS g_pPresentParameters = { };

	// handle window creation & destruction
	void CreateHWindow(const char* szWindowName);
	void DestroyHWindow();

	// handle device creation & destruction
	bool CreateDevice();
	void ResetDevice();
	void DestroyDevice();

	// handle ImGui creation & destruction
	void CreateImGui();
	void DestroyImGui();

	void BeginRender();
	void EndRender();
	void Render();
}