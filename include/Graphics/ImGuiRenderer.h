#pragma once
#include "DXDevice.h"

namespace Gino
{
	class ImGuiRenderer
	{
	public:
		ImGuiRenderer(HWND hwnd, const DevicePtr& dev, const DeviceContextPtr& ctx);
		~ImGuiRenderer();
		
		void BeginFrame();
		void EndFrame();

		const std::function<void(HWND, UINT, WPARAM, LPARAM)>& GetWin32Hook() const;

	private:
		std::function<void(HWND, UINT, WPARAM, LPARAM)> m_win32Hook;

	};
}


