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
		CentralRenderer(DXDevice* dxDev, bool vsync);
		~CentralRenderer();

		void Render(Model* model);

		// Exposed surface area to outside modules
		// This is required because we need to hook ImGui to the Window Proc for this applications main window!
		ImGuiRenderer* GetImGui() const;

	private:
		struct TestMipData
		{
			float mipLevel;
		};

		struct MVP
		{
			DirectX::SimpleMath::Matrix model;
			DirectX::SimpleMath::Matrix view;
			DirectX::SimpleMath::Matrix projection;
		};


	private:
		bool m_vsync;

		std::unique_ptr<ImGuiRenderer> m_imGui;

		Texture m_depth;
		DepthStencilStatePtr m_dss;

		DXDevice* m_dxDev;
		ShaderGroup m_shaderGroup;

		Texture m_mainTex;

		Material m_testMat;
		Model m_testModel;

		ConstantBuffer<TestMipData> m_cb;
		ConstantBuffer<MVP> m_mvpCB;

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
