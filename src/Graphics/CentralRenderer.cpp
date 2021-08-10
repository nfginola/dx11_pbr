#include "pch.h"
#include "Graphics/CentralRenderer.h"

namespace Gino
{
	// temporary
	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 normal;
	};

	CentralRenderer::CentralRenderer(DXDevice* dxDev) :
		m_dxDev(dxDev)
	{
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

		m_shaderGroup
			.AddStage(ShaderStage::Vertex, Utils::ReadFile("compiled_shaders/tri_vs.cso"))
			.AddStage(ShaderStage::Pixel, Utils::ReadFile("compiled_shaders/tri_ps.cso"))
			.AddInputDescs(Vertex_POS_UV_NORMAL::GetElementDescriptors())
			.Build(dev);

		// make vb and ib
		std::vector<Vertex_POS_UV_NORMAL> triVerts
		{
			{ { 0.f, 0.5f, 0.f }, { 1.f, 0.f }, { 0.f, 0.f, -1.f} },
			{ { 0.5f, -0.5f, 0.f }, { 0.f, 1.f }, { 0.f, 0.f, -1.f} },
			{ { -0.5f, -0.5f, 0.f }, { 1.f, 1.f }, { 0.f, 0.f, -1.f} }
		};
		std::vector<uint32_t> indices{ 0, 1, 2 };

		m_vb2.Initialize(dev, VertexBufferDesc<Vertex_POS_UV_NORMAL>{ .data = triVerts });
		m_ib2.Initialize(dev, IndexBufferDesc{ .data = indices });




		D3D11_RASTERIZER_DESC1 rsD
		{
			.FillMode = D3D11_FILL_SOLID,
			.CullMode = D3D11_CULL_BACK
		};
		HRCHECK(dev->CreateRasterizerState1(&rsD, m_rs.GetAddressOf()));


		// Test for resource cleanup warning signals
		// If we enable this code and let the code run and exit, we will see D3D11 memory leak since we dont release
		//ID3D11Buffer* tmpBuf;
		//HRCHECK(dev->CreateBuffer(&vbDesc, &vbDat, &tmpBuf));

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
		
		auto ctx = m_dxDev->GetContext();
		const float clearColor[4] = { 0.529f, 0.808f, 0.922f, 1.f };
		ctx->ClearRenderTargetView(m_dxDev->GetBackbufferView().Get(), clearColor);

		m_shaderGroup.Bind(ctx);

		ID3D11Buffer* vbs[] = { m_vb2.buffer.Get() };
		UINT vbStrides[] = { sizeof(Vertex_POS_UV_NORMAL) };
		UINT vbOffsets[] = { 0 };
		ctx->IASetVertexBuffers(0, _countof(vbs), vbs, vbStrides, vbOffsets);
		ctx->IASetIndexBuffer(m_ib2.buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		D3D11_VIEWPORT viewports[] = { m_dxDev->GetBackbufferViewport() };
		ctx->RSSetViewports(_countof(viewports), viewports);
		ctx->RSSetState(m_rs.Get());

		ID3D11RenderTargetView* rtvs[] = { m_dxDev->GetBackbufferView().Get() };
		ctx->OMSetRenderTargets(_countof(rtvs), rtvs, nullptr);

		//ctx->Draw(3, 0);
		ctx->DrawIndexedInstanced(3, 1, 0, 0, 0);

		m_dxDev->GetSwapChain()->Present(0, 0);
	}
}

