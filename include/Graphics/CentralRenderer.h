#pragma once
#include "DXDevice.h"
#include "ShaderGroup.h"
#include "ResourceTypes.h"

// Sub-renderers
#include "ImGuiRenderer.h"

namespace Gino
{
	class FPCamera;

	class CentralRenderer
	{
	public:
		CentralRenderer(DXDevice* dxDev, bool vsync);
		~CentralRenderer();

		void SetRenderCamera(FPCamera* cam);	// Primary user camera to use for rendering

		void Render(Model* model);				// Temporary model argument for testing purposes

		/*
		
		SubmitOpaque, do once. (Dont submit per frame)
		
		*/

		// Exposed surface area to outside modules
		// This is required because we need to hook ImGui to the Window Proc for this applications main window!
		ImGuiRenderer* GetImGui() const;

	private:
		struct TestMipData
		{
			float mipLevel;
		};
		
		// Should change to Per Frame, Per Pass, Per Material and Per Object constant buffers
		// Constant buffers by binding frequency!
		struct MVP
		{
			DirectX::SimpleMath::Matrix model;
			DirectX::SimpleMath::Matrix view;
			DirectX::SimpleMath::Matrix projection;
		};


	private:
		DXDevice* m_dxDev;
		std::unique_ptr<ImGuiRenderer> m_imGui;
		FPCamera* m_mainCamera;

		Texture m_backbuffer;

		bool m_vsync;

		ShaderGroup m_shaderGroup;

		ConstantBuffer<MVP> m_mvpCB;	

		// Swapchain framebuffer
		Framebuffer m_finalFramebuffer;

		Texture m_depth;

		// Render to texture
		Texture m_renderTexture;
		Framebuffer m_renderFramebuffer;
		ShaderGroup m_quadPass;
		SamplerStatePtr m_pointSampler;

		DepthStencilStatePtr m_dss;
		SamplerStatePtr m_mainSampler;
		RasterizerState1Ptr m_rs;



		/*
		// Dynamic allocations tho....
		std::vector<std::tuple<Mesh, Material, std::vector<Transform>> m_opaqueMeshes;
		std::vector<std::pair<Mesh, std::vector<Transform>> m_transparentMeshes;
		*/

	};
}
