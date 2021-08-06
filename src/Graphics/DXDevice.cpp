#include "pch.h"
#include "Graphics/DXDevice.h"

namespace Gino
{
	// Lets us print some debug messages sent to the queue
	// Currently responsible for printing WARNING debug messages to cout
	ComPtr<ID3D11InfoQueue> g_infoQueue;
	bool HRCHECK(HRESULT hr)
	{
		bool passed = SUCCEEDED(hr);

		// We have set a BREAK on CORRUPTION and ERRORS which are the major validators, no need to print out anything
		// If we do print out anything to the console, it'll be the warnings :)
		if (!passed && g_infoQueue != nullptr)
		{
			// https://stackoverflow.com/questions/53579283/directx-11-debug-layer-capture-error-strings
			HRESULT hr = g_infoQueue->PushEmptyStorageFilter();
			assert(hr == S_OK);

			UINT64 msgCount = g_infoQueue->GetNumStoredMessages();
			for (UINT64 i = 0; i < msgCount; i++) {

				// Enumerate
				SIZE_T msgSize = 0;
				g_infoQueue->GetMessage(i, nullptr, &msgSize); 
				
				// Fill
				D3D11_MESSAGE* message = (D3D11_MESSAGE*)malloc(msgSize); 
				hr = g_infoQueue->GetMessage(i, message, &msgSize);
				assert(hr == S_OK);
				
				// Log
				std::cout << message->pDescription << '\n';

				free(message);
			}
			g_infoQueue->ClearStoredMessages();
		}

		assert(passed);
		return passed;
	}

	DXDevice::DXDevice(HWND hwnd, int bbWidth, int bbHeight)
	{
		CreateDeviceAndContext();
		GetDebug();
		GetAvailableDisplayModes();
		CreateSwapchain(hwnd, bbWidth, bbHeight);
		CreateBackbufferRenderTarget();

		// Prevent DXGI from responding to Mode Changes and Alt + Enter (We will handle this ourselves)
		m_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
	}

	DXDevice::~DXDevice()
	{
		g_infoQueue.Reset();
		m_debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
	}

	DevicePtr DXDevice::GetDevice()
	{
		return m_device;
	}

	DeviceContextPtr DXDevice::GetContext()
	{
		return m_context;
	}

	SwapChainPtr DXDevice::GetSwapChain()
	{
		return m_swapChain;
	}

	RtvPtr DXDevice::GetBackbufferView()
	{
		return m_bbView;
	}

	void DXDevice::CreateDeviceAndContext()
	{
		std::array<D3D_FEATURE_LEVEL, 1> featureLevels{ D3D_FEATURE_LEVEL_11_0 };
		UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG;

		HRCHECK(
			D3D11CreateDevice(
				NULL,
				D3D_DRIVER_TYPE_HARDWARE,
				NULL,
				flags,
				featureLevels.data(),
				static_cast<UINT>(featureLevels.size()),
				D3D11_SDK_VERSION,
				m_device.GetAddressOf(),
				NULL,
				m_context.GetAddressOf()
			)
		);

		// Get factory associated with the created device
		ComPtr<IDXGIDevice> dxgiDev;
		HRCHECK(m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)dxgiDev.GetAddressOf()));
		HRCHECK(dxgiDev->GetParent(__uuidof(IDXGIAdapter), (void**)m_adapter.GetAddressOf()));
		m_adapter->GetParent(__uuidof(IDXGIFactory), (void**)m_factory.GetAddressOf());
	}

	void DXDevice::GetDebug()
	{
		HRCHECK(m_device->QueryInterface(__uuidof(ID3D11Debug), (void**)m_debug.GetAddressOf()));
		HRCHECK(m_debug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)g_infoQueue.GetAddressOf()));
		g_infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
		g_infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
		g_infoQueue->SetMuteDebugOutput(false);
	}

	void DXDevice::GetAvailableDisplayModes()
	{
		// DXGIOutput --> Represents an adapter output (e.g monitor)	
		ComPtr<IDXGIOutput> dxgiOutput;
		HRCHECK(m_adapter->EnumOutputs(0, dxgiOutput.GetAddressOf()));

		UINT numModes = 0;
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;	// Format to look for

		// Enumerate
		HRCHECK(dxgiOutput->GetDisplayModeList(format, 0, &numModes, NULL));

		// Fill list
		m_availableDisplayModes.resize(numModes);
		HRCHECK(dxgiOutput->GetDisplayModeList(format, 0, &numModes, m_availableDisplayModes.data()));
	}

	void DXDevice::CreateSwapchain(HWND hwnd, int bbWidth, int bbHeight)
	{
		// =============== Create Swapchain
		DXGI_MODE_DESC bestMode = m_availableDisplayModes.back();

		m_swapChainDesc.BufferDesc.Width = bbWidth;
		m_swapChainDesc.BufferDesc.Height = bbHeight;
		m_swapChainDesc.BufferDesc.RefreshRate.Numerator = bestMode.RefreshRate.Numerator;
		m_swapChainDesc.BufferDesc.RefreshRate.Denominator = bestMode.RefreshRate.Denominator;
		m_swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		m_swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
		m_swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		// No MSAA enabled for now
		m_swapChainDesc.SampleDesc.Quality = 0;
		m_swapChainDesc.SampleDesc.Count = 1;

		// Surface usage is back buffer!
		m_swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
		m_swapChainDesc.BufferCount = 2;		// One front and one back buffer

		m_swapChainDesc.OutputWindow = hwnd;
		m_swapChainDesc.Windowed = true;
		m_swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		// Allow switch mode through IDXGISwapChain::ResizeTarget (e.g Windowed to Fullscreen)
		//m_scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		// We handle exclusive fullscreen manually :)
		m_swapChainDesc.Flags = 0;

		HRCHECK(m_factory->CreateSwapChain(m_device.Get(), &m_swapChainDesc, m_swapChain.GetAddressOf()));
	}

	void DXDevice::CreateBackbufferRenderTarget()
	{
		// ============= Create Backbuffer View
		HRCHECK(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)m_bbTex.GetAddressOf()));

		// Gamma corrected when writing into backbuffer view
		// We cannot use SRGB on the SwapChain format since it's not allowed when using the new FLIP model
		// https://walbourn.github.io/care-and-feeding-of-modern-swapchains/
		// "sRGB gamma/de-gamma behavior is really an aspect of “interpreting the bits”, i.e. part of the view."
		// "When you read a pixel from your typical r8g8b8a8 texture, you need to remove the gamma curve, do all your lighting math, 
		// then apply the gamma curve as you write the pixel into your render target"
		// --> SRGB on SRV read automatically makes sure to de-gamma it before use
		// --> SRGB on RTV write automatically makes sure to gamma it before writing
		//D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		//rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;		// Automatically gamma correct result to our non-SRBG backbuffer
		//rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		//HRCHECK(m_device->CreateRenderTargetView(m_bbTex.Get(), &rtvDesc, m_bbView.GetAddressOf()));

		// We will use default UNORM non-SRGB specified by Texture format (We gamma correct manually)
		HRCHECK(m_device->CreateRenderTargetView(m_bbTex.Get(), nullptr, m_bbView.GetAddressOf()));	
	}
}

