#include "pch.h"
#include "Graphics/Renderer.h"

#include "FPCamera.h"
#include "Graphics/Model.h"


namespace Gino
{
	Renderer::Renderer(DXDevice* dxDev, bool vsync) :
		m_mainCamera(nullptr),
		m_vsync(vsync),
		m_dxDev(dxDev),
		m_imGui(std::make_unique<ImGuiRenderer>(dxDev->GetHWND(), dxDev->GetDevice(), dxDev->GetContext()))
	{

		// make swapchain framebuffer
		m_finalFramebuffer.Initialize({ m_dxDev->GetBackbufferTarget() });

		std::cout << "vsync: " << (vsync ? "on" : "off") << '\n';
 
		auto dev = dxDev->GetDevice();
		auto ctx = dxDev->GetContext();

		m_forwardOpaqueShaders
			.AddStage(ShaderStage::Vertex, "compiled_shaders/tri_vs.cso")
			.AddStage(ShaderStage::Pixel, "compiled_shaders/tri_ps.cso")
			.AddInputDescs(Vertex_POS_UV_NORMAL::GetElementDescriptors())
			.Build(dev);

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


		// make depth stencil state (closely tied to the depth stencil view, essentially configs for writing to the DSV)
		D3D11_DEPTH_STENCIL_DESC dssDesc
		{
			.DepthEnable = true,
			.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
			.DepthFunc = D3D11_COMPARISON_LESS_EQUAL,
			.StencilEnable = false
		};
		HRCHECK(dev->CreateDepthStencilState(&dssDesc, m_dss.GetAddressOf()));
		
		// MVP CB
		m_mvpCB.Initialize(dev);



		// Setup HDR render to texture
		D3D11_TEXTURE2D_DESC renderTexDesc
		{
			.Width = (uint32_t)m_dxDev->GetBackbufferViewport().Width,
			.Height = (uint32_t)m_dxDev->GetBackbufferViewport().Height,
			.MipLevels = 1,
			.ArraySize = 1,
			.Format = DXGI_FORMAT_R16G16B16A16_FLOAT,		// Floating point 16bit for HDR
			.SampleDesc = {.Count = 1, .Quality = 0 },
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
			.CPUAccessFlags = 0,
			.MiscFlags = 0
		};
		m_renderTexture.Initialize(dev, ctx, renderTexDesc);

		m_renderFramebuffer.Initialize({ m_renderTexture.GetRTV() }, m_depth.GetDSV());

		m_fullscreenQuadShaders
			.AddStage(ShaderStage::Vertex, "compiled_shaders/quadpass_vs.cso")
			.AddStage(ShaderStage::Pixel, "compiled_shaders/quadpass_ps.cso")
			.Build(dev);

		// make point sampler
		D3D11_SAMPLER_DESC pointSSDesc
		{
			.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
			.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
			.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
			.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
			.MipLODBias = 0.f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL,
			.BorderColor = { 0.f, 0.f, 0.f, 1.f },
			.MinLOD = 0.f,
			.MaxLOD = 0.f
		};
		HRCHECK(dev->CreateSamplerState(&pointSSDesc, m_pointSampler.GetAddressOf()));

	}

	Renderer::~Renderer()
	{
	}

	void Renderer::SetRenderCamera(FPCamera* cam)
	{
		m_mainCamera = cam;
	}

	void Renderer::AddOpaqueModel(Model* model)
	{
		assert(false);
	}
	
	void Renderer::Render(Model* model)
	{
		assert(m_mainCamera != nullptr);	// A render camera is required!
		auto ctx = m_dxDev->GetContext();
		m_imGui->BeginFrame();

		m_mvpCB.data.model = DirectX::SimpleMath::Matrix::CreateScale(0.07f);	// Specific to sponza
		m_mvpCB.data.view = m_mainCamera->GetViewMatrix();
		m_mvpCB.data.projection = m_mainCamera->GetProjectionMatrix();
		m_mvpCB.Upload(ctx);
		ctx->VSSetConstantBuffers(0, 1, m_mvpCB.buffer.GetAddressOf());

		m_forwardOpaqueShaders.Bind(ctx);

		ID3D11SamplerState* samplers[] = { m_mainSampler.Get() };
		ctx->PSSetSamplers(0, 1, samplers);

		// set rasterizer state
		D3D11_VIEWPORT viewports[] = { m_dxDev->GetBackbufferViewport() };
		ctx->RSSetViewports(_countof(viewports), viewports);
		ctx->RSSetState(m_rs.Get());

		// Render to texture
		m_renderFramebuffer.Clear(ctx, { { 0.529f, 0.808f, 0.922f, 1.f } });
		m_renderFramebuffer.Bind(ctx);
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
			assert(meshes.size() == materials.size());
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

		// Unbind framebuffer so that we can read textures associated with it
		m_renderFramebuffer.Unbind(ctx);


		// Render fullscreen quad pass
		// Input --> Render texture to read from and framebuffer to render to
		{
			m_fullscreenQuadShaders.Bind(ctx);
			m_finalFramebuffer.Clear(ctx);
			m_finalFramebuffer.Bind(ctx);

			// Bind previous framebuffer render content for reading
			ID3D11ShaderResourceView* srvs[] = { m_renderTexture.GetSRV() };
			ctx->PSSetShaderResources(0, 1, srvs);

			// Point sample
			ID3D11SamplerState* smplrs[] = { m_pointSampler.Get() };
			ctx->PSSetSamplers(0, 1, smplrs);

			// Draw quad
			ctx->Draw(6, 0);

			// Unbind texture so that it can be reused for writing
			ID3D11ShaderResourceView* nullSRVs[] = { nullptr };
			ctx->PSSetShaderResources(0, 1, nullSRVs);
		}



		// Draw UI directly on swapchain (no filtering or anything applied on it)
		m_imGui->EndFrame(ctx, m_finalFramebuffer);
		m_dxDev->GetSwapChain()->Present(m_vsync ? 1 : 0, 0);
	}

	ImGuiRenderer* Renderer::GetImGui() const
	{
		return m_imGui.get();
	}

}

