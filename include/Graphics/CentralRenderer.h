#pragma once
#include "DXDevice.h"
#include "ShaderGroup.h"
#include "ResourceTypes.h"

// Sub-renderers
#include "ImGuiRenderer.h"

namespace Gino
{
	class CentralRenderer
	{
	public:
		CentralRenderer(DXDevice* dxDev, ImGuiRenderer* imGui, bool vsync);
		~CentralRenderer();

		void Render();

	private:
		struct TestMipData
		{
			float mipLevel;
		};

	private:
		bool m_vsync;

		ImGuiRenderer* m_imGui;

		DXDevice* m_dxDev;
		ShaderGroup m_shaderGroup;

		Texture m_mainTex;

		Material m_testMat;
		Model m_testModel;

		ConstantBuffer<TestMipData> m_cb;

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
