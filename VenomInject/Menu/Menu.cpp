#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <format>

#include <Windows.h>
#include <TlHelp32.h>

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include "Menu.h"
#include "../Utilities/Utilities.h"
#include "../Injector/Injector.h"
#include "Vars.h"

#pragma comment(lib, "d3d9.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter
);

long long __stdcall WindowProcess(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg)
	{
	case WM_SIZE: {
		if (Menu::g_pDevice && wParam != SIZE_MINIMIZED)
		{
			Menu::g_pPresentParameters.BackBufferWidth = LOWORD(lParam);
			Menu::g_pPresentParameters.BackBufferHeight = HIWORD(lParam);
			Menu::ResetDevice();
		}
	}return 0;

	case WM_SYSCOMMAND: {
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
	}break;

	case WM_DESTROY: {
		PostQuitMessage(0);
	}return 0;

	case WM_LBUTTONDOWN: {
		Menu::g_Position = MAKEPOINTS(lParam); // set click points
	}return 0;

	case WM_MOUSEMOVE: {
		if (wParam == MK_LBUTTON)
		{
			const auto points = MAKEPOINTS(lParam);
			auto rect = ::RECT{ };

			GetWindowRect(Menu::g_hWnd, &rect);

			rect.left += points.x - Menu::g_Position.x;
			rect.top += points.y - Menu::g_Position.y;

			if (Menu::g_Position.x >= 0 &&
				Menu::g_Position.x <= Menu::WIDTH &&
				Menu::g_Position.y >= 0 && Menu::g_Position.y <= 19)
				SetWindowPos(
					Menu::g_hWnd,
					HWND_TOPMOST,
					rect.left,
					rect.top,
					0, 0,
					SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER
				);
		}

	}return 0;

	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Menu::CreateHWindow(const char* windowName)
{
	g_hWndClass.cbSize = sizeof(WNDCLASSEX);
	g_hWndClass.style = CS_CLASSDC;
	g_hWndClass.lpfnWndProc = WindowProcess;
	g_hWndClass.cbClsExtra = 0;
	g_hWndClass.cbWndExtra = 0;
	g_hWndClass.hInstance = GetModuleHandleA(0);
	g_hWndClass.hIcon = 0;
	g_hWndClass.hCursor = 0;
	g_hWndClass.hbrBackground = 0;
	g_hWndClass.lpszMenuName = 0;
	g_hWndClass.lpszClassName = L"class001";
	g_hWndClass.hIconSm = 0;

	RegisterClassEx(&g_hWndClass);

	g_hWnd = CreateWindowExA(
		0,
		"class001",
		windowName,
		WS_POPUP,
		100,
		100,
		WIDTH,
		HEIGHT,
		0,
		0,
		g_hWndClass.hInstance,
		0
	);

	ShowWindow(g_hWnd, SW_SHOWDEFAULT);
	UpdateWindow(g_hWnd);
}

void Menu::DestroyHWindow()
{
	DestroyWindow(g_hWnd);
	UnregisterClass(g_hWndClass.lpszClassName, g_hWndClass.hInstance);
}

bool Menu::CreateDevice()
{
	g_pD3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!g_pD3d)
		return false;

	ZeroMemory(&g_pPresentParameters, sizeof(g_pPresentParameters));

	g_pPresentParameters.Windowed = TRUE;
	g_pPresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_pPresentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	g_pPresentParameters.EnableAutoDepthStencil = TRUE;
	g_pPresentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	g_pPresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (g_pD3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		g_hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&g_pPresentParameters,
		&g_pDevice) < 0)
		return false;

	return true;
}

void Menu::ResetDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = g_pDevice->Reset(&g_pPresentParameters);

	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();
}

void Menu::DestroyDevice()
{
	if (g_pDevice)
	{
		g_pDevice->Release();
		g_pDevice = nullptr;
	}

	if (g_pD3d)
	{
		g_pD3d->Release();
		g_pD3d = nullptr;
	}
}

void Menu::CreateImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ::ImGui::GetIO();

	io.IniFilename = NULL;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(g_hWnd);
	ImGui_ImplDX9_Init(g_pDevice);
}

void Menu::DestroyImGui()
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Menu::BeginRender()
{
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);

		if (message.message == WM_QUIT)
		{
			g_bIsRunning = !g_bIsRunning;
			return;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void Menu::EndRender()
{
	ImGui::EndFrame();

	g_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	g_pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	g_pDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (g_pDevice->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		g_pDevice->EndScene();
	}

	const auto result = g_pDevice->Present(0, 0, 0, 0);

	// Handle loss of D3D9 device
	if (result == D3DERR_DEVICELOST && g_pDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}

struct ProcessInfo_t
{
	DWORD pid;
	std::wstring name;
	DWORD threadCount; // Added thread count
};

// Updated process retrieval function
std::vector<ProcessInfo_t> GetProcessList()
{
	std::vector<ProcessInfo_t> processes;

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return processes;

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnapshot, &pe32))
	{
		do
		{
			processes.push_back({ pe32.th32ProcessID, pe32.szExeFile, pe32.cntThreads });
		} while (Process32Next(hSnapshot, &pe32));
	}

	CloseHandle(hSnapshot);
	return processes;
}

void Menu::Render()
{
	static std::vector<ProcessInfo_t> allProcesses = GetProcessList();
	static std::vector<ProcessInfo_t> vecFilteredProcesses = allProcesses;

	static std::vector<std::string> addedDlls;  // List of DLLs that the user added
	static char szDllPathInput[1024] = "";     // Input buffer for new DLL path

	static char szFilterText[128] = "";        // Filter input buffer
	static std::string previousFilter = "";

	constexpr ImGuiWindowFlags nFlags = ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove;

	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ 1100.0f, 600.0f }); // Adjust window size as needed

	if (ImGui::Begin("VenomInject", &g_bIsRunning, nFlags))
	{
		if (ImGui::BeginTabBar("MainTabBar"))
		{
			if (ImGui::BeginTabItem("Main"))
			{
				// Filter input
				ImGui::InputText("Filter", szFilterText, IM_ARRAYSIZE(szFilterText));
				ImGui::SameLine();

				// Refresh process list
				if (ImGui::Button("Refresh"))
				{
					allProcesses = GetProcessList();
					vecFilteredProcesses = allProcesses; // Reset filtered list
				}

				// Apply filtering logic when filter text changes
				if (previousFilter != szFilterText)
				{
					previousFilter = szFilterText;
					vecFilteredProcesses.clear();

					for (const auto& process : allProcesses)
					{
						std::wstring nameLower = process.name;
						std::wstring filterLower = std::wstring(szFilterText, szFilterText + strlen(szFilterText));

						// Convert to lowercase for case-insensitive comparison
						std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::towlower);
						std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::towlower);

						if (nameLower.find(filterLower) != std::wstring::npos)
							vecFilteredProcesses.push_back(process);
					}
				}

				// DLL Path input and Add button
				ImGui::InputText("DLL Path", szDllPathInput, sizeof(szDllPathInput));
				ImGui::SameLine();
				// Open File Explorer Dialog
				if (ImGui::Button("Browse"))
				{
					std::string selectedPath = Utilities::OpenFileExplorerDialog();
					if (!selectedPath.empty())
					{
						strncpy_s(szDllPathInput, selectedPath.c_str(), sizeof(szDllPathInput) - 1);
						szDllPathInput[sizeof(szDllPathInput) - 1] = '\0'; // Ensure null termination
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Add DLL"))
				{
					if (strlen(szDllPathInput) > 0)
					{
						addedDlls.push_back(std::string(szDllPathInput));
						szDllPathInput[0] = '\0'; // Clear input after adding
					}
				}

				// Layout adjustment to display tables side by side
				ImGui::BeginGroup(); // Group for layout

				// Start Process Table on the left
				ImGui::BeginChild("ProcessTableChild", ImVec2(400, 300), true); // Fixed size for ProcessTable
				if (ImGui::BeginTable("ProcessTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
				{
					// Process Table Columns: PID and Process Name
					ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 80.0f);  // Fixed width for PID column
					ImGui::TableSetupColumn("Process Name", ImGuiTableColumnFlags_WidthFixed, 200.0f);  // Reduced width for Process Name column
					ImGui::TableHeadersRow();

					// Iterate over filtered processes and render rows
					for (size_t i = 0; i < vecFilteredProcesses.size(); ++i)
					{
						const auto& process = vecFilteredProcesses[i];

						ImGui::TableNextRow();

						// PID column
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("%d", process.pid);

						// Process Name column
						ImGui::TableSetColumnIndex(1);
						ImGui::TextUnformatted(std::string(process.name.begin(), process.name.end()).c_str());
					}

					ImGui::EndTable();
				}
				ImGui::EndChild(); // End Process Table

				ImGui::SameLine(); // Place DllTable to the right of the ProcessTable

				// Start DLL Table on the right
				ImGui::BeginChild("DllTableChild", ImVec2(500, 300), true); // Fixed size for DllTable
				if (ImGui::BeginTable("DllTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
				{
					// Add columns for DLL Path and Platform
					ImGui::TableSetupColumn("DLL Path", ImGuiTableColumnFlags_WidthStretch);
					ImGui::TableSetupColumn("Platform", ImGuiTableColumnFlags_WidthFixed, 100.0f); // Fixed width for Platform column
					ImGui::TableHeadersRow();

					// Display added DLLs in the table
					for (size_t i = 0; i < addedDlls.size(); ++i)
					{
						ImGui::TableNextRow();

						// DLL Path column
						ImGui::TableSetColumnIndex(0);
						ImGui::TextUnformatted(addedDlls[i].c_str());

						// Platform column
						ImGui::TableSetColumnIndex(1);
						std::string platform = Utilities::GetDllPlatform(addedDlls[i].c_str());
						ImGui::TextUnformatted(platform.c_str());
					}

					ImGui::EndTable();
				}
				ImGui::EndChild(); // End DLL Table

				ImGui::EndGroup(); // End grouping for layout

				ImGui::EndTabItem();
			}

			// Settings Tab
			if (ImGui::BeginTabItem("Settings"))
			{
				ImGui::Combo("Injection Method", reinterpret_cast<int*>(&Vars::g_nInjectionMethod), Injector::g_szMethodNames, Injector::GetMethodCount());
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();
	}
}