#include "pch.h"
#include "Engine.h"
#include "DXDevice.h"

namespace Gino
{
	Engine::Engine(Settings& settings)
	{
		m_dxDev = std::make_unique<DXDevice>(settings.hwnd, settings.resolutionWidth, settings.resolutionHeight);
		
		

	}

	Engine::~Engine()
	{

	}

	void Engine::SimulateAndRender()
	{
		const float clearColor[4] = { 0.529f, 0.808f, 0.922f, 1.f };
		m_dxDev->GetContext()->ClearRenderTargetView(m_dxDev->GetBackbufferView().Get(), clearColor);



		m_dxDev->GetSwapChain()->Present(0, 0);
	}
	//Input* Engine::GetInput()
	//{
	//	return m_input.get();
	//}
}


