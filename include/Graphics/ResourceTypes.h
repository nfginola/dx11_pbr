#pragma once
#include <vector>
#include "DXDevice.h"
#include "ShaderGroup.h"

namespace Gino
{
	struct Vertex_POS_UV_NORMAL
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 normal;

		static std::vector<D3D11_INPUT_ELEMENT_DESC> GetElementDescriptors();
	};

	struct DepthStencilClearDesc
	{
		UINT clearFlags;		// D3D11_CLEAR_FLAG 
		FLOAT depth;
		UINT8 stencil;
	};

	struct Framebuffer
	{
	public:
		Framebuffer();
		~Framebuffer();
		
		void Initialize(std::array<RtvPtr, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> targets, DsvPtr dsv = nullptr);

		void Clear(const DeviceContextPtr& ctx, const std::array<FLOAT[4], D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> clearValues, const DepthStencilClearDesc& dsClearDesc = {});
		void Bind(const DeviceContextPtr& ctx);
		
		// In case of OMSetRenderTargetsAndUnorderedAccessViews
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

	struct StructuredBufferDesc
	{
		int a;		// To be extended
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

		/*
		Stride,
		Offset,
		etc. (Put in when we need it: We will need it for Mesh loading)
		
		*/
	
		Buffer() = default;
		~Buffer() = default;

		void Initialize(const DevicePtr& dev, const D3D11_BUFFER_DESC& desc, const void* initData);
		
		template <typename T>
		void Initialize(const DevicePtr& dev, VertexBufferDesc<T> desc);			// Templated version for ease of use
		void Initialize(const DevicePtr& dev, VertexBufferDescRaw desc);
		void Initialize(const DevicePtr& dev, IndexBufferDesc desc);
		void Initialize(const DevicePtr& dev, StructuredBufferDesc desc);
		void Initialize(const DevicePtr& dev, RWBufferDesc desc);
		void Initialize(const DevicePtr& dev, ByteAddressBufferDesc desc);
		void Initialize(const DevicePtr& dev, IndirectArgsBufferDesc desc);
	};

	template <typename T>
	struct ConstantBuffer
	{
		BufferPtr buffer;
		T data;

		ConstantBuffer();
		~ConstantBuffer();

		void Initialize(const DevicePtr& dev, const std::vector<T>& initData, bool dynamic, bool cpuUpdatable);
		void Upload(const DeviceContextPtr& ctx);
	};
	
	template<typename T>
	inline ConstantBuffer<T>::ConstantBuffer()
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

		D3D11_SUBRESOURCE_DATA data{};
		data.pSysMem = initData.data();
		HRCHECK(dev->CreateBuffer(&desc, &data, buffer.GetAddressOf()));
	}

	template<typename T>
	inline void ConstantBuffer<T>::Upload(const DeviceContextPtr& ctx)
	{
		D3D11_MAPPED_SUBRESOURCE* subres;
		ctx->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
		std::memcpy(subres, &data, sizeof(T));
		ctx->Unmap(buffer);
	}

	template<typename T>
	inline void Buffer::Initialize(const DevicePtr& dev, VertexBufferDesc<T> desc)
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

	// We will continue on this as soon as we have textures up and running
	//struct Texture
	//{
	//	


	//};




}

