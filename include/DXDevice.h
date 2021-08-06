#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <assert.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

using SwapChainPtr = ComPtr<IDXGISwapChain>;
using DevicePtr = ComPtr<ID3D11Device>;
using DeviceContextPtr = ComPtr<ID3D11DeviceContext>;
using DebugPtr = ComPtr<ID3D11Debug>;
using FactoryPtr = ComPtr<IDXGIFactory>;

// Common resources
using Tex2DPtr = ComPtr<ID3D11Texture2D>;
using BufferPtr = ComPtr<ID3D11Buffer>;

// Views
using SrvPtr = ComPtr<ID3D11ShaderResourceView>;
using RtvPtr = ComPtr<ID3D11RenderTargetView>;
using UavPtr = ComPtr<ID3D11UnorderedAccessView>;
using DsvPtr = ComPtr<ID3D11DepthStencilView>;
using AdapterPtr = ComPtr<IDXGIAdapter>;

namespace Gino
{
	static bool HRCHECK(HRESULT hr);

	class DXDevice
	{
	public:
		DXDevice(HWND hwnd, int bbWidth, int bbHeight);
		~DXDevice();

		DevicePtr GetDevice();
		DeviceContextPtr GetContext();
		SwapChainPtr GetSwapChain();
		RtvPtr GetBackbufferView();

	private:
		void CreateDeviceAndContext();
		void GetDebug();
		void GetAvailableDisplayModes();
		void CreateSwapchain(HWND hwnd, int bbWidth, int bbHeight);
		void CreateBackbufferRenderTarget();

	private:
		DevicePtr m_device;
		DeviceContextPtr m_context;
		AdapterPtr m_adapter;
		FactoryPtr m_factory;
		DebugPtr m_debug;

		SwapChainPtr m_swapChain;
		DXGI_SWAP_CHAIN_DESC m_swapChainDesc;
		Tex2DPtr m_bbTex;
		RtvPtr m_bbView;

		std::vector<DXGI_MODE_DESC> m_availableDisplayModes;

	};
}


