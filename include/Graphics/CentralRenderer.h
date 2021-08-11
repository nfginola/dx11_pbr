#pragma once
#include "DXDevice.h"
#include "ShaderGroup.h"
#include "ResourceTypes.h"

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
		ShaderGroup m_shaderGroup;

		Buffer m_vb;
		Buffer m_ib;
		Texture m_mainTex;

		SamplerStatePtr m_mainSampler;

		Framebuffer m_finalFramebuffer;

		RasterizerState1Ptr m_rs;

		/*
		// Dynamic allocations tho....
		std::vector<std::tuple<Mesh, Material, std::vector<Transform>> m_opaqueMeshes;
		std::vector<std::pair<Mesh, std::vector<Transform>> m_transparentMeshes;
		*/

	};
}
