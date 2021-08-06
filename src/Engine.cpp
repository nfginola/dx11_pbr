#include "pch.h"
#include "Engine.h"
#include "Graphics/DXDevice.h"
#include "Graphics/CentralRenderer.h"

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
		
		culler->cull(scene)
		
		for each non-culled geometry in scene:
			cr->SubmitOpaqueModel(mesh, material);
			cr->SubmitTransparentModel(mesh, material);
		
		*/

		m_centralRenderer->Render();
	}

	//Input* Engine::GetInput()
	//{
	//	return m_input.get();
	//}
}


