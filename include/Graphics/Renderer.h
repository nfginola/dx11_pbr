#pragma once
#include "DXDevice.h"
#include "ShaderGroup.h"
#include "ResourceTypes.h"

// Sub-renderers
#include "ImGuiRenderer.h"

#include "Component.h"		// Needs to know Transform Component

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
		void SetModels(const std::vector<std::pair<Model*, std::vector<Transform*>>>* models);


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

		struct CB_PerFrame
		{
			DirectX::SimpleMath::Matrix view;
			DirectX::SimpleMath::Matrix projection;
		};

		struct CB_PerObject
		{
			DirectX::SimpleMath::Matrix model;
		};
		
		// Should change to Per Frame, Per Pass, Per Material and Per Object constant buffers
		// Constant buffers by binding frequency!
		struct MVP
		{
			DirectX::SimpleMath::Matrix model;
			DirectX::SimpleMath::Matrix view;
			DirectX::SimpleMath::Matrix projection;
		};

	// Things to render
	private:
		const std::vector<std::pair<Model*, std::vector<Transform*>>>* m_opaqueModels; // Current scene data

	// Render resources
	private:
		std::unique_ptr<ImGuiRenderer> m_imGui;

		bool m_vsync;
		DXDevice* m_dxDev;
		FPCamera* m_mainCamera;

		ConstantBuffer<CB_PerFrame> m_cbPerFrame;

		// Swapchain framebuffer
		Framebuffer m_finalFramebuffer;
		Texture m_backbuffer;

		// Model draw pass
		ShaderGroup m_forwardOpaqueShaders;
		//ConstantBuffer<MVP> m_mvpCB;	
		ConstantBuffer<CB_PerObject> m_cbPerObject;
		Buffer m_instanceBuffer;
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
