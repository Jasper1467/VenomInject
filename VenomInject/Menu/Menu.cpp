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

void RenderFilterAndRefresh(std::vector<ProcessInfo_t>& vecAllProcesses,
	std::vector<ProcessInfo_t>& vecFilteredProcesses,
	char szFilterText[128],
	std::string& previousFilter)
{
	ImGui::InputText("Filter", szFilterText, IM_ARRAYSIZE(szFilterText));
	ImGui::SameLine();

	if (ImGui::Button("Refresh"))
	{
		vecAllProcesses = GetProcessList();
		vecFilteredProcesses = vecAllProcesses; // Reset filtered list
	}

	// Apply filtering logic
	if (previousFilter != szFilterText)
	{
		previousFilter = szFilterText;
		vecFilteredProcesses.clear();

		for (const auto& process : vecAllProcesses)
		{
			std::wstring nameLower = process.name;
			std::wstring filterLower = std::wstring(szFilterText, szFilterText + strlen(szFilterText));

			std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::towlower);
			std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::towlower);

			if (nameLower.find(filterLower) != std::wstring::npos)
				vecFilteredProcesses.push_back(process);
		}
	}
}

void RenderDllInput(std::vector<std::string>& addedDlls, char szDllPathInput[1024])
{
	if (ImGui::InputTextWithHint("##DLLPath", "Enter DLL Path...", szDllPathInput, sizeof(szDllPathInput), ImGuiInputTextFlags_EnterReturnsTrue))
	{
		if (strlen(szDllPathInput) > 0)
		{
			addedDlls.push_back(std::string(szDllPathInput));
			szDllPathInput[0] = '\0'; // Clear input
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Browse"))
	{
		std::string selectedPath = Utilities::OpenFileExplorerDialog();
		if (!selectedPath.empty())
		{
			addedDlls.push_back(selectedPath);
		}
	}
}

void RenderProcessTable(const std::vector<ProcessInfo_t>& vecFilteredProcesses, int& selectedProcessIndex)
{
	ImGui::BeginChild("ProcessTableChild", ImVec2(400, 300), true);
	if (ImGui::BeginTable("ProcessTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
	{
		ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 80.0f);
		ImGui::TableSetupColumn("Process Name", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();

		for (size_t i = 0; i < vecFilteredProcesses.size(); ++i)
		{
			const auto& process = vecFilteredProcesses[i];

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			if (ImGui::Selectable(std::to_string(process.pid).c_str(), selectedProcessIndex == i, ImGuiSelectableFlags_SpanAllColumns))
			{
				selectedProcessIndex = i;
			}

			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(std::string(process.name.begin(), process.name.end()).c_str());
		}
		ImGui::EndTable();
	}
	ImGui::EndChild();
}

void RenderDllTable(std::vector<std::string>& addedDlls)
{
	ImGui::BeginChild("DllTableChild", ImVec2(500, 300), true);
	if (ImGui::BeginTable("DllTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("DLL Path", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Platform", ImGuiTableColumnFlags_WidthFixed, 100.0f);
		ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 80.0f);
		ImGui::TableHeadersRow();

		for (size_t i = 0; i < addedDlls.size(); ++i)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(addedDlls[i].c_str());

			ImGui::TableSetColumnIndex(1);
			std::string platform = Utilities::GetDllPlatform(addedDlls[i].c_str());
			ImGui::TextUnformatted(platform.c_str());

			ImGui::TableSetColumnIndex(2);
			if (ImGui::Button(("Delete##" + std::to_string(i)).c_str()))
			{
				addedDlls.erase(addedDlls.begin() + i);
				break;
			}
		}
		ImGui::EndTable();
	}
	ImGui::EndChild();
}

void RenderInjectionButton(const std::vector<ProcessInfo_t>& vecFilteredProcesses, int selectedProcessIndex, const std::vector<std::string>& addedDlls)
{
	if (ImGui::Button("Inject DLL") && selectedProcessIndex >= 0 && !addedDlls.empty())
	{
		const auto& selectedProcess = vecFilteredProcesses[selectedProcessIndex];
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, selectedProcess.pid);

		if (hProcess)
		{
			Injector::Inject(hProcess, addedDlls, (Injector::Method_e)Vars::g_nInjectionMethod);
			CloseHandle(hProcess);
		}
	}
}

void RenderSettingsTab()
{
	ImGui::Combo("Injection Method", reinterpret_cast<int*>(&Vars::g_nInjectionMethod), Injector::g_szMethodNames, Injector::GetMethodCount());
}

void Menu::Render()
{
	static std::vector<ProcessInfo_t> vecAllProcesses = GetProcessList();
	static std::vector<ProcessInfo_t> vecFilteredProcesses = vecAllProcesses;

	static std::vector<std::string> addedDlls;  // List of DLLs that the user added
	static char szDllPathInput[1024] = "";      // Input buffer for new DLL path
	static char szFilterText[128] = "";         // Filter input buffer

	static std::string previousFilter = "";
	static int selectedProcessIndex = -1;

	constexpr ImGuiWindowFlags nFlags = ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove;

	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ 1100.0f, 600.0f });

	if (ImGui::Begin("VenomInject", &g_bIsRunning, nFlags))
	{
		if (ImGui::BeginTabBar("MainTabBar"))
		{
			if (ImGui::BeginTabItem("Main"))
			{
				// Render the Filter and Refresh UI
				RenderFilterAndRefresh(vecAllProcesses, vecFilteredProcesses, szFilterText, previousFilter);

				// Render the DLL input
				RenderDllInput(addedDlls, szDllPathInput);

				ImGui::BeginGroup(); // Grouping for layout purposes

				// Render Process Table
				RenderProcessTable(vecFilteredProcesses, selectedProcessIndex);

				ImGui::Spacing();

				// Render the "Inject DLL" button
				RenderInjectionButton(vecFilteredProcesses, selectedProcessIndex, addedDlls);

				ImGui::EndGroup(); // End layout group

				ImGui::SameLine();

				// Render DLL Table
				RenderDllTable(addedDlls);

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Settings"))
			{
				RenderSettingsTab();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();
	}
}