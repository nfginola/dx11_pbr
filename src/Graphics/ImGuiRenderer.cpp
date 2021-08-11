#include "pch.h"
#include "Graphics/ImGuiRenderer.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Gino
{
	ImGuiRenderer::ImGuiRenderer(HWND hwnd, const DevicePtr& dev, const DeviceContextPtr& ctx)
	{
		// Init ImGUI
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplDX11_Init(dev.Get(), ctx.Get());

		m_win32Hook = [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			{
				return true;
			}
		};
	}

	ImGuiRenderer::~ImGuiRenderer()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiRenderer::BeginFrame()
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		bool showDemoWindow = true;
		ImGui::ShowDemoWindow(&showDemoWindow);
	}

	void ImGuiRenderer::EndFrame()
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
	
	const std::function<void(HWND, UINT, WPARAM, LPARAM)>& ImGuiRenderer::GetWin32Hook() const
	{
		return m_win32Hook;
	}
}
