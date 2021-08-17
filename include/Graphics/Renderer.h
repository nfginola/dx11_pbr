#pragma once
#include "DXDevice.h"
#include "ShaderGroup.h"
#include "ResourceTypes.h"

// Sub-renderers
#include "ImGuiRenderer.h"

namespace Gino
{
	class FPCamera;
	class Model;

	class Renderer
	{
	public:
		Renderer(DXDevice* dxDev, bool vsync);
		~Renderer();

		void SetRenderCamera(FPCamera* cam);	// Primary user camera to use for rendering

		void AddOpaqueModel(Model* model);

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
		//std::vector<Model*> m_opaqueModels;

	// Render resources
	private:
		std::unique_ptr<ImGuiRenderer> m_imGui;

		bool m_vsync;
		DXDevice* m_dxDev;
		FPCamera* m_mainCamera;

		// Swapchain framebuffer
		Framebuffer m_finalFramebuffer;
		Texture m_backbuffer;

		// Model draw pass
		ShaderGroup m_forwardOpaqueShaders;
		ConstantBuffer<MVP> m_mvpCB;	
		Framebuffer m_renderFramebuffer;
		Texture m_depth;
		Texture m_renderTexture;

		DepthStencilStatePtr m_dss;
		SamplerStatePtr m_mainSampler;
		RasterizerState1Ptr m_rs;

		// Render to quad (with renderFramebuffer as input)
		ShaderGroup m_fullscreenQuadShaders;
		SamplerStatePtr m_pointSampler;


	};
}
