#pragma once
#include "DXDevice.h"
#include "ResourceTypes.h"

namespace Gino
{
	class FPCamera;

	class SkyboxRenderer
	{
	public:
		struct CB_CameraData
		{
			DirectX::SimpleMath::Matrix viewProj;
		};

	public:
		SkyboxRenderer(DXDevice* dev);
		~SkyboxRenderer() = default;

		void SetCamera(FPCamera* camera);
		void Render(Framebuffer& framebuffer, const D3D11_VIEWPORT& vp);

	private:
		DXDevice* m_dxDev;

		FPCamera* m_activeCam;
		ShaderGroup m_autoCubeShader;
		ConstantBuffer<CB_CameraData> m_cbCam;

		SamplerStatePtr m_skyboxSampler;
		RasterizerState1Ptr m_rss;
		DepthStencilStatePtr m_dss;
		Texture m_skyboxTex;
	};
}


