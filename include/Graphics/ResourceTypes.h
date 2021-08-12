#pragma once
#include <vector>
#include <variant>
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

	private:
		void CreateViews(const DevicePtr& dev, const DeviceContextPtr& ctx, const D3D11_TEXTURE2D_DESC& desc);

	private:
		Tex2DPtr m_texture;	
		
		SrvPtr m_srv;
		RtvPtr m_rtv;
		DsvPtr m_dsv;
		UavPtr m_uav;

		


	};



	// ====================== Higher level abstractions

	enum class MaterialType
	{
		PHONG,
		PBR
	};

	struct PhongMaterialData
	{
		Texture* m_diffuse;
		Texture* m_specular;
		Texture* m_opacity;
		Texture* m_normal;

		// Other misc. data (e.g colors) can be stored here too
	};

	struct PBRMaterialData
	{
		Texture* m_albedo;
		Texture* m_normal;
		Texture* m_metallic;
		Texture* m_roughness;
		Texture* m_ao;

		// Other misc. data can be stored here too
	};

	// We use variant to simplify the data pipeline for our application needs
	// Idea is: Branch the rendering by material type --> If type is PBR, go to PBR renderer, etc.
	struct Material
	{
	public:
		void Initialize(const PhongMaterialData& data);
		void Initialize(const PBRMaterialData& data);

		template <typename T>
		const T& GetProperties() const;

		MaterialType GetType() const;
	private:
		MaterialType m_type;
		std::variant<PhongMaterialData, PBRMaterialData> m_data;
		// ShaderGroup and other Pipeline resources? Maybe not?

	};

	// Represents offsets into a Mesh (VB/IB) that represents a specific mesh part of a model for drawing
	struct MeshPart
	{
		uint32_t indicesFirstIndex;		// First index in IB
		uint32_t numIndices;			// Vertex count to draw
		uint32_t vertexOffset;			// First index in VB

		Material* material;				// Shading information for this part
	};

	// A collection of meshes that represents a coherent geometric model
	struct Mesh
	{
		Buffer vb;
		Buffer ib;
		std::vector<MeshPart> submeshes;
	};

	template<typename T>
	inline const T& Material::GetProperties() const
	{
		try
		{
			return std::get<T>(m_data);
		}
		catch (std::bad_variant_access const& ex)
		{
			std::cout << ex.what() << " || Gino::Material : GetProperties() is called with an invalid type!\n";
			assert(false);

			// Should never reach here: Here to get rid of the warnings.. 
			return std::get<T>(m_data);
		}
	}

}

