#pragma once
#include <vector>
#include <variant>
#include <utility>
#include "DXDevice.h"
#include "SimpleMath.h"		// Must be included AFTER <d3d11.h>/<DirectXMath.h> (SimpleMath depends on DirectXMath)
#include "ShaderGroup.h"


namespace Gino
{
	struct Vertex_POS_UV_NORMAL
	{
		DirectX::SimpleMath::Vector3 pos;
		DirectX::SimpleMath::Vector2 uv;
		DirectX::SimpleMath::Vector3 normal;

		static std::vector<D3D11_INPUT_ELEMENT_DESC> GetElementDescriptors();
	};

	// depth and stencil clear flags set automatically, override if needed
	struct DepthStencilClearDesc
	{
		FLOAT depth = 1.f;
		UINT8 stencil = 0;		// Default stencil clear value
		UINT clearFlags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL;		// D3D11_CLEAR_FLAG 
	};

	struct Framebuffer
	{
	public:
		Framebuffer();
		~Framebuffer();
		
		void Initialize(std::array<RtvPtr, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> targets, DsvPtr dsv = nullptr);

		void Clear(const DeviceContextPtr& ctx, const std::array<FLOAT[4], D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> clearValues = { 0.f, 0.f, 0.f, 1.f }, const DepthStencilClearDesc& dsClearDesc = {});
		void Bind(const DeviceContextPtr& ctx);						// RTV bind (writing)
		void Unbind(const DeviceContextPtr& ctx);
		
		// For OMSetRenderTargetsAndUnorderedAccessViews usage
		// Otherwise we can have another Bind function for the Framebuffer which instead takes in UAVs
		// e.g --> BindWithUAVs(ctx, UAVs)
		const std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> GetRenderTargets() const;
		const DsvPtr& GetDepthStencilView() const;

	private:
		std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> m_renderTargets;
		uint32_t m_activeRenderTargets;
		DsvPtr m_depthStencilView;
	};
	
	struct VertexBufferDescRaw
	{
		const void* data;
		size_t totalSize;
	};

	template <typename T>
	struct VertexBufferDesc
	{
		std::vector<T> data;
	};

	struct IndexBufferDesc
	{
		std::vector<uint32_t> data;
	};

	template <typename T>
	struct StructuredBufferDesc
	{
		size_t elementCount;
		bool dynamic;
		bool cpuWrite;
		std::vector<T> data;
		
	};

	// Special type of Structured Buffer
	struct RWBufferDesc
	{
		int a;		// To be extended	
	};

	// Special type of Structured Buffer
	struct ByteAddressBufferDesc
	{
		int a;		// To be extended
	};

	// For draw-commands filled on GPU side
	struct IndirectArgsBufferDesc
	{
		int a;		// To be extended
	};

	// Other buffer types
	struct Buffer
	{
		BufferPtr buffer;

		SrvPtr srv;
		UavPtr uav;

		/*
		Stride,
		Offset,
		etc. (Put in when we need it: We will need it for Mesh loading)
		
		*/
	
		Buffer() = default;
		~Buffer() = default;

		void Initialize(const DevicePtr& dev, const D3D11_BUFFER_DESC& desc, const void* initData);
		
		template <typename T>
		void Initialize(const DevicePtr& dev, const VertexBufferDesc<T>& desc);			// Templated version for ease of use
		void Initialize(const DevicePtr& dev, const VertexBufferDescRaw& desc);
		void Initialize(const DevicePtr& dev, const IndexBufferDesc& desc);

		template <typename T>
		void Initialize(const DevicePtr& dev, const StructuredBufferDesc<T>& desc);
		void Initialize(const DevicePtr& dev, const RWBufferDesc& desc);
		void Initialize(const DevicePtr& dev, const ByteAddressBufferDesc& desc);
		void Initialize(const DevicePtr& dev, const IndirectArgsBufferDesc& desc);
		void Initialize(const DevicePtr& dev, const D3D11_BUFFER_DESC& desc);

	private:
		void CreateViews(const DevicePtr& dev, const D3D11_BUFFER_DESC& desc);

	}; 

	template <typename T>
	struct ConstantBuffer
	{
		BufferPtr buffer;
		T data;

		ConstantBuffer();
		~ConstantBuffer();

		void Initialize(const DevicePtr& dev, const std::vector<T>& initData = {}, bool dynamic = true, bool cpuUpdatable = true);
		void Upload(const DeviceContextPtr& ctx);
	};
	
	template<typename T>
	inline ConstantBuffer<T>::ConstantBuffer() :
		data({})
	{
	}

	template<typename T>
	inline ConstantBuffer<T>::~ConstantBuffer()
	{
	}

	template<typename T>
	inline void ConstantBuffer<T>::Initialize(const DevicePtr& dev, const std::vector<T>& initData, bool dynamic, bool cpuUpdatable)
	{
		D3D11_BUFFER_DESC desc{};
		desc.ByteWidth = sizeof(T) + (16 - (sizeof(T) % 16));	// Always 16 bytes align the buffer
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		if (dynamic && cpuUpdatable)
		{
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		else if (dynamic && !cpuUpdatable)
		{
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.CPUAccessFlags = 0;
		}
		else if (!dynamic && !cpuUpdatable)
		{
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.CPUAccessFlags = 0;
		}
		else
		{
			// Cant be cpuUpdatable but not dynamic..
			assert(false);
		}

		if (initData.empty())
		{
			HRCHECK(dev->CreateBuffer(&desc, nullptr, buffer.GetAddressOf()));
		}
		else
		{
			D3D11_SUBRESOURCE_DATA data{};
			data.pSysMem = initData.data();
			HRCHECK(dev->CreateBuffer(&desc, &data, buffer.GetAddressOf()));
		}
	}

	template<typename T>
	inline void ConstantBuffer<T>::Upload(const DeviceContextPtr& ctx)
	{
		D3D11_MAPPED_SUBRESOURCE subres;
		ctx->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
		std::memcpy(subres.pData, &data, sizeof(T));
		ctx->Unmap(buffer.Get(), 0);
	}


	template<typename T>
	inline void Buffer::Initialize(const DevicePtr& dev, const VertexBufferDesc<T>& desc)
	{
		D3D11_BUFFER_DESC vbDesc
		{
			.ByteWidth = static_cast<uint32_t>(desc.data.size()) * sizeof(T),		// narrowing conversion exists
			.Usage = D3D11_USAGE_IMMUTABLE,
			.BindFlags = D3D11_BIND_VERTEX_BUFFER
		};
		D3D11_SUBRESOURCE_DATA vbDat
		{
			.pSysMem = desc.data.data(),
			.SysMemPitch = vbDesc.ByteWidth
		};
		HRCHECK(dev->CreateBuffer(&vbDesc, &vbDat, buffer.GetAddressOf()));
	}

	template<typename T>
	inline void Buffer::Initialize(const DevicePtr& dev, const StructuredBufferDesc<T>& desc)
	{
		size_t totalSize = sizeof(T) * desc.elementCount;
		size_t alignedTotalSize = totalSize + (sizeof(T) - (totalSize % sizeof(T)));      // must be multiple of structuedByteStride!

		D3D11_USAGE usage;
		UINT cpuAccessFlags = 0;

		if (desc.cpuWrite && desc.dynamic) // gpu read, cpu write
		{
			usage = D3D11_USAGE_DYNAMIC;
			cpuAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		else if (!desc.cpuWrite && desc.dynamic)
		{
			usage = D3D11_USAGE_DEFAULT;
			cpuAccessFlags = 0;
		}
		else
		{
			usage = D3D11_USAGE_IMMUTABLE;
			cpuAccessFlags = 0;
		}

		D3D11_BUFFER_DESC sbDesc
		{
			.ByteWidth = (uint32_t)alignedTotalSize,
			.Usage = usage,
			.BindFlags = D3D11_BIND_SHADER_RESOURCE,
			.CPUAccessFlags = cpuAccessFlags,
			.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
			.StructureByteStride = sizeof(T)
		};

		if (desc.data.empty())
		{
			// no init data
			HRCHECK(dev->CreateBuffer(&sbDesc, nullptr, buffer.GetAddressOf()));
		}
		else
		{
			D3D11_SUBRESOURCE_DATA subres{ .pSysMem = desc.data.data() };
			HRCHECK(dev->CreateBuffer(&sbDesc, &subres, buffer.GetAddressOf()));
		}

		CreateViews(dev, sbDesc);
	}

	// Subject to change (adding 1D/3D)
	struct Texture
	{
	public:
		ID3D11Texture2D* GetTexture() const;		// We will use getter in case of future changes (adding 1D/3D textures)

		void Initialize(const DevicePtr& dev, const DeviceContextPtr& ctx, const D3D11_TEXTURE2D_DESC& desc, Utils::ImageData* imageData = nullptr);
		ID3D11ShaderResourceView* GetSRV() const;
		ID3D11RenderTargetView* GetRTV() const;
		ID3D11DepthStencilView* GetDSV() const;
		ID3D11UnorderedAccessView* GetUAV() const;

		void InitializeFromFile(const DevicePtr& dev, const DeviceContextPtr& ctx, const std::filesystem::path& filePath, bool srgb = true, bool genMipMaps = true);
		void InitializeFromExisting(const Tex2DPtr& tex, const RtvPtr& rtv = nullptr, const SrvPtr& srv = nullptr, const DsvPtr& dsv = nullptr, const UavPtr& uav = nullptr);

	private:
		void CreateViews(const DevicePtr& dev, const DeviceContextPtr& ctx, const D3D11_TEXTURE2D_DESC& desc);

	private:
		Tex2DPtr m_texture;	
		
		SrvPtr m_srv;
		RtvPtr m_rtv;
		DsvPtr m_dsv;
		UavPtr m_uav;


	};



}

