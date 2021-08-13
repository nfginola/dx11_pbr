#include "pch.h"
#include "Graphics/CentralRenderer.h"
#include "FPCamera.h"

namespace Gino
{
	CentralRenderer::CentralRenderer(DXDevice* dxDev, bool vsync) :
		m_vsync(vsync),
		m_dxDev(dxDev),
		m_imGui(std::make_unique<ImGuiRenderer>(dxDev->GetHWND(), dxDev->GetDevice(), dxDev->GetContext()))
	{
		std::cout << "vsync: " << (vsync ? "on" : "off") << '\n';
 
		/*
		
		For automatically avoiding rebinding already bound resources:
			We can call it DXStateCache
		
		*/

		auto dev = dxDev->GetDevice();
		auto ctx = dxDev->GetContext();

		// make shaders
		m_shaderGroup
			.AddStage(ShaderStage::Vertex, "compiled_shaders/tri_vs.cso")
			.AddStage(ShaderStage::Pixel, "compiled_shaders/tri_ps.cso")
			.AddInputDescs(Vertex_POS_UV_NORMAL::GetElementDescriptors())
			.Build(dev);


		// make rasterizer state
		D3D11_RASTERIZER_DESC1 rsD
		{
			.FillMode = D3D11_FILL_SOLID,
			.CullMode = D3D11_CULL_BACK
		};
		HRCHECK(dev->CreateRasterizerState1(&rsD, m_rs.GetAddressOf()));
	
		// make sampler
		D3D11_SAMPLER_DESC samplerDesc
		{
			.Filter = D3D11_FILTER_ANISOTROPIC,
			.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
			.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
			.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
			.MipLODBias = 0.f,
			.MaxAnisotropy = 8,								// We have to check max anisotropy available, but lets set to 8
			.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL,		// Not used.. what do we do with the information on whether it has passed?? // Read more
			.BorderColor = { 0.f, 0.f, 0.f, 1.f },
			.MinLOD = 0.f,
			.MaxLOD = D3D11_FLOAT32_MAX
		};
		HRCHECK(dev->CreateSamplerState(&samplerDesc, m_mainSampler.GetAddressOf()));

		// make depth buffer
		D3D11_TEXTURE2D_DESC depthDesc
		{
			.Width = (uint32_t)m_dxDev->GetBackbufferViewport().Width,
			.Height = (uint32_t)m_dxDev->GetBackbufferViewport().Height,
			.MipLevels = 1,
			.ArraySize = 1,
			.Format = DXGI_FORMAT_D32_FLOAT,		// NO STENCIL, otherwise use DXGI_FORMAT_D24_UNORM_S8_UINT
			.SampleDesc = {.Count = 1, .Quality = 0 },
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_DEPTH_STENCIL,
			.CPUAccessFlags = 0,
			.MiscFlags = 0
		};
		m_depth.Initialize(dev, ctx, depthDesc);

		// make framebuffer
		m_finalFramebuffer.Initialize({ m_dxDev->GetBackbufferTarget() }, m_depth.GetDSV());

		// make depth stencil state (closely tied to the depth stencil view, essentially configs for writing to the DSV)
		D3D11_DEPTH_STENCIL_DESC dssDesc
		{
			.DepthEnable = true,
			.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
			.DepthFunc = D3D11_COMPARISON_LESS_EQUAL,
			.StencilEnable = false
		};
		HRCHECK(dev->CreateDepthStencilState(&dssDesc, m_dss.GetAddressOf()));


		// Try cb
		m_cb.Initialize(dev);
		
		// Mvp cb
		m_mvpCB.Initialize(dev);

		// Test for resource cleanup warning signals
		// If we enable this code and let the code run and exit, we will see D3D11 memory leak since we dont release
		//ID3D11Buffer* tmpBuf;
		//D3D11_BUFFER_DESC vbDesc
		//{
		//	.ByteWidth = 32,
		//	.Usage = D3D11_USAGE_DYNAMIC,
		//	.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		//	.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE
		//};
		//HRCHECK(dev->CreateBuffer(&vbDesc, NULL, &tmpBuf));
	}

	CentralRenderer::~CentralRenderer()
	{
	}

	void CentralRenderer::SetMainCamera(FPCamera* cam)
	{
		m_mainCamera = cam;
	}
	
	static float timeElapsed = 0.f;
	void CentralRenderer::Render(Model* model)
	{
		/*
		
		shadowFramebuffer = shadowPass->run(scene, meshes)
		msFramebuffer = forwardPass->run(shadowFrameBuffer, meshes, material)

		resolveFramebuffer(msFramebuffer, resolvedFramebuffer, format);
	
		// This should finally render to the swapchain directly 
		// Internally, it may do some copies since we may do some PostProcess through a PS (tonemapping)
		// downsample and then use a compute shader to do a two-pass gaussian blur and then upsample
		postProcessPass->run(resolveFramebuffer, finalRenderTarget);
		
		*/

		timeElapsed += 0.002f;


		m_imGui->BeginFrame();
		
		auto ctx = m_dxDev->GetContext();


		// Test update using constant buffer
		float mipLevel = cosf(timeElapsed) * 5.f + 5.f;
		//std::cout << "mip: " << mipLevel << "\n";
		m_cb.data.mipLevel = mipLevel;
		m_cb.Upload(ctx);
		ctx->PSSetConstantBuffers(0, 1, m_cb.buffer.GetAddressOf());

		// MVP
		//m_mvpCB.data.model =
		//	DirectX::SimpleMath::Matrix::CreateScale(0.07f);

		//m_mvpCB.data.view =
		//	DirectX::XMMatrixLookAtLH({ 0.f, 2.f, 0.f }, { -4.f, 10.f - mipLevel, 0.f }, { 0.f, 1.f, 0.f });

		//m_mvpCB.data.projection =
		//	DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(80.f), 16.f / 9.f, 0.1f, 1000.f);

		m_mvpCB.data.model = DirectX::SimpleMath::Matrix::CreateScale(0.07f);
		m_mvpCB.data.view = m_mainCamera->GetViewMatrix();
		m_mvpCB.data.projection = m_mainCamera->GetProjectionMatrix();
		m_mvpCB.Upload(ctx);
		ctx->VSSetConstantBuffers(0, 1, m_mvpCB.buffer.GetAddressOf());


		m_shaderGroup.Bind(ctx);

		ID3D11SamplerState* samplers[] = { m_mainSampler.Get() };
		ctx->PSSetSamplers(0, 1, samplers);

		// set rasterizer state
		D3D11_VIEWPORT viewports[] = { m_dxDev->GetBackbufferViewport() };
		ctx->RSSetViewports(_countof(viewports), viewports);
		ctx->RSSetState(m_rs.Get());

		// set OM state
		m_finalFramebuffer.Clear(ctx, 
			{ { 0.529f, 0.808f, 0.922f, 1.f } }, 
			DepthStencilClearDesc{ .clearFlags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, .depth = 1.f, .stencil = 1 });
		m_finalFramebuffer.Bind(ctx);
		ctx->OMSetDepthStencilState(m_dss.Get(), 0);

		// Test draw model
		{
			ID3D11Buffer* vbs[] = { model->GetVB() };
			UINT vbStrides[] = { sizeof(Vertex_POS_UV_NORMAL) };
			UINT vbOffsets[] = { 0 };
			ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			ctx->IASetVertexBuffers(0, _countof(vbs), vbs, vbStrides, vbOffsets);
			ctx->IASetIndexBuffer(model->GetIB(), DXGI_FORMAT_R32_UINT, 0);
			ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			auto meshes = model->GetMeshes();
			auto materials = model->GetMaterials();
			for (uint32_t i = 0; i < meshes.size(); ++i)
			{
				ID3D11ShaderResourceView* srvs[] = 
				{ 
					materials[i].GetProperties<PhongMaterialData>().diffuse->GetSRV() ,
					materials[i].GetProperties<PhongMaterialData>().specular->GetSRV(),
					materials[i].GetProperties<PhongMaterialData>().normal->GetSRV(),
					materials[i].GetProperties<PhongMaterialData>().opacity->GetSRV()
				};
				ctx->PSSetShaderResources(0, 4, srvs);
				ctx->DrawIndexedInstanced(meshes[i].numIndices, 1, meshes[i].indicesFirstIndex, meshes[i].vertexOffset, 0);
			}

		}




		m_imGui->EndFrame(ctx, m_finalFramebuffer);

		m_dxDev->GetSwapChain()->Present(m_vsync ? 1 : 0, 0);
	}

	ImGuiRenderer* CentralRenderer::GetImGui() const
	{
		return m_imGui.get();
	}

}

