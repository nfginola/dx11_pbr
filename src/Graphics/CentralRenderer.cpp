#include "pch.h"
#include "Graphics/CentralRenderer.h"



namespace Gino
{
	CentralRenderer::CentralRenderer(DXDevice* dxDev) :
		m_dxDev(dxDev),
		m_imGui(std::make_unique<ImGuiRenderer>(dxDev->GetHWND(), dxDev->GetDevice().Get(), dxDev->GetContext().Get()))
	{
		/*
		
		For automatically avoiding rebinding already bound resources:
			We can call it DXStateCache
		
		*/

		/*

			We should make one Buffer class, which can take in multiple different descriptors of our choice instead of having multiple classes.
			We simply follow the spirit of D3D11 when it comes to strongly typed vs non-strongly typed (in this case, there is only a ID3D11Buffer for all types of buffer)

			struct ConstantBufferDesc
			struct RWBufferDesc
			struct UAVDesc
			etc..

			Although we should probably employ a single Texture class in line with Vulkan style (take in 1D, 2D, 3D descriptors) ??
			We may have to read a bit more about the functionalities provided to textures.. (whether they differ between 1D, 2D and 3D)	

		*/
		auto dev = dxDev->GetDevice();
		auto ctx = dxDev->GetContext();

		// make shaders
		m_shaderGroup
			.AddStage(ShaderStage::Vertex, "compiled_shaders/tri_vs.cso")
			.AddStage(ShaderStage::Pixel, "compiled_shaders/tri_ps.cso")
			.AddInputDescs(Vertex_POS_UV_NORMAL::GetElementDescriptors())
			.Build(dev);

		// make vb and ib
		std::vector<Vertex_POS_UV_NORMAL> triVerts
		{
			{ { 0.f, 0.5f, 0.f }, { 0.5f, 0.f }, { 0.f, 0.f, -1.f} },
			{ { 0.5f, -0.5f, 0.f }, { 1.f, 1.f }, { 0.f, 0.f, -1.f} },
			{ { -0.5f, -0.5f, 0.f }, { 0.f, 1.f }, { 0.f, 0.f, -1.f} }
		};
		std::vector<uint32_t> indices{ 0, 1, 2 };

		m_vb.Initialize(dev, VertexBufferDesc<Vertex_POS_UV_NORMAL>{ .data = triVerts });
		m_ib.Initialize(dev, IndexBufferDesc{ .data = indices });

		// make framebuffer
		m_finalFramebuffer.Initialize({ m_dxDev->GetBackbufferTarget() });

		// make texture
		m_mainTex.InitializeFromFile(dev, ctx, "../assets/scenery.jpg");

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

	void CentralRenderer::Render()
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


		m_imGui->BeginFrame();
		
		auto ctx = m_dxDev->GetContext();

		m_shaderGroup.Bind(ctx);

		ID3D11Buffer* vbs[] = { m_vb.buffer.Get() };
		UINT vbStrides[] = { sizeof(Vertex_POS_UV_NORMAL) };
		UINT vbOffsets[] = { 0 };
		ctx->IASetVertexBuffers(0, _countof(vbs), vbs, vbStrides, vbOffsets);
		ctx->IASetIndexBuffer(m_ib.buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		D3D11_VIEWPORT viewports[] = { m_dxDev->GetBackbufferViewport() };
		ctx->RSSetViewports(_countof(viewports), viewports);
		ctx->RSSetState(m_rs.Get());

		//ID3D11ShaderResourceView* srvs[] = { m_texView.Get() };
		ID3D11ShaderResourceView* srvs[] = { m_mainTex.GetSRV() };
		ctx->PSSetShaderResources(0, 1, srvs);
		ID3D11SamplerState* samplers[] = { m_mainSampler.Get() };
		ctx->PSSetSamplers(0, 1, samplers);

		m_finalFramebuffer.Clear(ctx, { { 0.529f, 0.808f, 0.922f, 1.f } });
		m_finalFramebuffer.Bind(ctx);

		ctx->DrawIndexedInstanced(3, 1, 0, 0, 0);

		
		m_imGui->EndFrame();

		m_dxDev->GetSwapChain()->Present(0, 0);
	}

	ImGuiRenderer* CentralRenderer::GetImGui() const
	{
		if (m_imGui)
		{
			return m_imGui.get();
		}
		else
		{
			return nullptr;
		}
	}

}

