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
		auto dev = dxDev->GetDevice();
		
		// We probably want a DXBinder to avoid unneccessary API calls.
		// We simply check for equivalence for all things we bind, simply mirroring the API state on the CPU!
		// We use this Binder to bind every API call along with helper functions that we see fit!
		

		// Lets not mind duplicate shader creations for now 
		// - Shader Group: 
		// Create Vertex Shader
		// Create Pixel Shader
		// Create Input Layout		-- Validates with Vertex Shader bytecode (has to have VS compiled binary data)
		// Create Input Topology	// set with context

		/*
			Create a normal triangle CW (D3D standard) with Position, UV and Normal with VB/IB
		*/
		// Create and Populate VB/IB
		// Create and set Rasterizer State (CW front-face)

		auto vsBin = Utils::ReadFile("compiled_shaders/tri_vs.cso");
		auto psBin = Utils::ReadFile("compiled_shaders/tri_ps.cso");

		HRCHECK(dev->CreateVertexShader(vsBin.data(), vsBin.size(), nullptr, m_vs.GetAddressOf()));
		HRCHECK(dev->CreatePixelShader(psBin.data(), psBin.size(), nullptr, m_ps.GetAddressOf()));
		
		// D3D11_APPEND_ALIGNED_ELEMENT includes packing (padding) for us!
		D3D11_INPUT_ELEMENT_DESC inputDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		HRCHECK(dev->CreateInputLayout(inputDescs, _countof(inputDescs), vsBin.data(), vsBin.size(), m_inputLayout.GetAddressOf()));

		Vertex triVerts[] =
		{
			{ { 0.f, 0.5f, 0.f }, { 1.f, 0.f }, { 0.f, 0.f, -1.f} },
			{ { 0.5f, -0.5f, 0.f }, { 0.f, 1.f }, { 0.f, 0.f, -1.f} },
			{ { -0.5f, -0.5f, 0.f }, { 1.f, 1.f }, { 0.f, 0.f, -1.f} }
		};

		D3D11_BUFFER_DESC vbDesc{};
		vbDesc.ByteWidth = sizeof(triVerts);
		vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
		D3D11_SUBRESOURCE_DATA vbDat{};
		vbDat.pSysMem = triVerts;
		vbDat.SysMemPitch = vbDesc.ByteWidth;
		HRCHECK(dev->CreateBuffer(&vbDesc, &vbDat, m_vb.GetAddressOf()));
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

		ctx->IASetInputLayout(m_inputLayout.Get());
		ID3D11Buffer* vbs[] = { m_vb.Get() };
		UINT vbStrides[] = { sizeof(Vertex) };
		UINT vbOffsets[] = { 0 };
		ctx->IASetVertexBuffers(0, _countof(vbs), vbs, vbStrides, vbOffsets);
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		ctx->VSSetShader(m_vs.Get(), nullptr, 0);
		ctx->PSSetShader(m_ps.Get(), nullptr, 0);

		D3D11_VIEWPORT vp{};
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		vp.MinDepth = 0.f;
		vp.MaxDepth = 1.f;
		// Match the backbuffer dimension
		vp.Width = 1920;
		vp.Height = 1080;
		D3D11_VIEWPORT viewports[] = { vp };
		ctx->RSSetViewports(_countof(viewports), viewports);

		ID3D11RenderTargetView* rtvs[] = { m_dxDev->GetBackbufferView().Get() };
		ctx->OMSetRenderTargets(_countof(rtvs), rtvs, nullptr);

		ctx->Draw(3, 0);

		m_dxDev->GetSwapChain()->Present(0, 0);
	}
}

