#include "pch.h"
#include "Graphics/SkyboxRenderer.h"
#include "FPCamera.h"

namespace Gino
{

	SkyboxRenderer::SkyboxRenderer(DXDevice* dxDev) :
		m_dxDev(dxDev),
		m_activeCam(nullptr)
	{
		auto& dev = dxDev->GetDevice();
		auto& ctx = dxDev->GetContext();
		m_cbCam.Initialize(dev);

		D3D11_SAMPLER_DESC samplerDesc
		{
			.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
			.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
			.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
			.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
			.MipLODBias = 0.f,
			.MaxAnisotropy = 0,								
			.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL,		
			.BorderColor = { 0.f, 0.f, 0.f, 1.f },
			.MinLOD = 0.f,
			.MaxLOD = D3D11_FLOAT32_MAX
		};
		HRCHECK(dev->CreateSamplerState(&samplerDesc, m_skyboxSampler.GetAddressOf()));

		m_autoCubeShader
			.AddStage(ShaderStage::Vertex, "compiled_shaders/Skybox_VS.cso")
			.AddStage(ShaderStage::Pixel, "compiled_shaders/Skybox_PS.cso")
			.Build(dev);


		// skybox test
		m_skyboxTex.InitializeCubeFromFile(dev, ctx,
			{
				"../assets/textures/skyboxes/yokohama/posx.jpg",
				"../assets/textures/skyboxes/yokohama/negx.jpg",
				"../assets/textures/skyboxes/yokohama/posy.jpg",
				"../assets/textures/skyboxes/yokohama/negy.jpg",
				"../assets/textures/skyboxes/yokohama/posz.jpg",
				"../assets/textures/skyboxes/yokohama/negz.jpg"
			}
		);

		// rasterizer state so we can see cube from inside
		D3D11_RASTERIZER_DESC1 rssDesc
		{
			.FillMode = D3D11_FILL_SOLID,
			.CullMode = D3D11_CULL_BACK,
			.FrontCounterClockwise = true,	// look from inside (d3d11 default is front clockwise = true)
			.DepthBias = 0,
			.DepthBiasClamp = 0.f,
			.SlopeScaledDepthBias = 0.f,
			.DepthClipEnable = true
		};

		HRCHECK(dev->CreateRasterizerState1(&rssDesc, m_rss.GetAddressOf()));

		// depth stencil state
		D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;		// dont write to depth stencil buffer

		HRCHECK(dev->CreateDepthStencilState(&dsDesc, m_dss.GetAddressOf()));
	}

	void SkyboxRenderer::SetCamera(FPCamera* camera)
	{
		m_activeCam = camera;
	}

	void SkyboxRenderer::Render(Framebuffer& framebuffer, const D3D11_VIEWPORT& vp)
	{
		assert(m_activeCam != nullptr);

		auto& ctx = m_dxDev->GetContext();

		m_cbCam.data.viewProj = m_activeCam->GetViewMatrix() * m_activeCam->GetProjectionMatrix();
		m_cbCam.Upload(ctx);
		ctx->VSSetConstantBuffers(0, 1, m_cbCam.buffer.GetAddressOf());

		// Set pipeline states
		ctx->IASetInputLayout(nullptr);
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_autoCubeShader.Bind(ctx);
		ID3D11ShaderResourceView* srvs[] = { m_skyboxTex.GetSRV() };
		ctx->PSSetShaderResources(0, _countof(srvs), srvs);
		ctx->PSSetSamplers(0, 1, m_skyboxSampler.GetAddressOf());

		D3D11_VIEWPORT vps[] = { vp };
		ctx->RSSetViewports(_countof(vps), vps);
		ctx->RSSetState(m_rss.Get());

		ctx->OMSetDepthStencilState(m_dss.Get(), 0);

		// Draw cube with immediate buffer
		framebuffer.Bind(ctx);
		ctx->Draw(36, 0);
		framebuffer.Unbind(ctx);
	}


}
