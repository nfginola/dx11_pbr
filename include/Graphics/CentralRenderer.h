#pragma once
#include "DXDevice.h"
#include "ShaderGroup.h"

namespace Gino
{
	class CentralRenderer
	{
	public:
		CentralRenderer(DXDevice* dxDev);
		~CentralRenderer();

		void Render();

	private:
		DXDevice* m_dxDev;

		VsPtr m_vs;
		PsPtr m_ps;
		ShaderGroup m_shaderGroup;

		InputLayoutPtr m_inputLayout;
		BufferPtr m_vb;

		/*
		// Dynamic allocations tho....
		std::vector<std::tuple<Mesh, Material, std::vector<Transform>> m_opaqueMeshes;
		std::vector<std::pair<Mesh, std::vector<Transform>> m_transparentMeshes;
		*/

	};
}
