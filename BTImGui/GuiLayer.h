#pragma once
#include "imgui.h"
#include "imgui/backends/imgui_impl_dx9.h"
#include "imgui/backends/imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>
#include <string>
class GuiLayer
{
public:
	std::string m_loginMessage;
	LPDIRECT3D9              g_pD3D = nullptr;
	LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
	D3DPRESENT_PARAMETERS    g_d3dpp = {};
	bool CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();
	void ResetDevice();
	GuiLayer() = default;
	void Initialize();
};

