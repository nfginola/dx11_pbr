#include "pch.h"
#include "Engine.h"
#include "Graphics/DXDevice.h"
#include "Graphics/CentralRenderer.h"
#include "AssimpLoader.h"

namespace Gino
{
	Engine::Engine(Settings& settings)
	{
		m_dxDev = std::make_unique<DXDevice>(settings.hwnd, settings.resolutionWidth, settings.resolutionHeight);

		// DXState state
		/*
			this should PRIMARILY be used inside CentralRenderer and other Render modules
			state->Set(...)
			state->FinalizeBinds();		--> All API calls in one go
		*/

		m_centralRenderer = std::make_unique<CentralRenderer>(m_dxDev.get());

	}

	Engine::~Engine()
	{

	}

	void Engine::SimulateAndRender()
	{

		/*
		
		UpdateObjects(dt)

		culler->cull(scene)
		
		for each non-culled geometry in scene:
			cr->SubmitOpaqueModel(mesh, material);
			cr->SubmitTransparentModel(mesh, material);
		
		*/

		m_centralRenderer->Render();
	}

	std::function<void(HWND, UINT, WPARAM, LPARAM)> Engine::GetImGuiHook() const
	{
		if (m_centralRenderer && m_centralRenderer->GetImGui())
		{
			return m_centralRenderer->GetImGui()->GetWin32Hook();
		}
		else
		{
			return[](HWND, UINT, WPARAM, LPARAM) {};
		}
	}

	//Input* Engine::GetInput()
	//{
	//	return m_input.get();
	//}
}


