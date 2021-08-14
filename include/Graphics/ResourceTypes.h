#pragma once
#include <vector>
#include <variant>
#include <utility>
#include "DXDevice.h"
#include "SimpleMath.h"		// Must be included AFTEr <d3d11.h>
#include "ShaderGroup.h"


namespace Gino
{
	struct Vertex_POS_UV_NORMAL
	{
		DirectX::SimpleMath::Vector3 pos;
		DirectX::SimpleMath::Vector2  uv;
		DirectX::SimpleMath::Vector3  normal;

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
		Phong,
		PBR
	};

	struct PhongMaterialData
	{
		Texture* diffuse = nullptr;
		Texture* specular = nullptr;
		Texture* opacity = nullptr;
		Texture* normal = nullptr;

		// Other misc. data (e.g colors) can be stored here too
	};

	struct PBRMaterialData
	{
		Texture* albedo = nullptr;
		Texture* normal = nullptr;
		Texture* metallic = nullptr;
		Texture* roughness = nullptr;
		Texture* ao = nullptr;

		// Other misc. data can be stored here too
	};

	// We use variant to make it simple. No complex material schemes.
	// Idea here is that we can easily modify and extend this should we want other material types 
	// - We add another initialize that takes in another Data type
	// - Add another ENUM
	// - Existing materials undisturbed!
	struct Material
	{
	public:
		Material() = default;
		~Material() = default;

		void Initialize(const PhongMaterialData& data);
		void Initialize(const PBRMaterialData& data);

		template <typename T>
		const T& GetProperties() const;

		MaterialType GetType() const;
	private:
		MaterialType m_type = MaterialType::Phong;
		std::variant<PhongMaterialData, PBRMaterialData> m_data;
		// ShaderGroup and other Pipeline resources? Maybe not?

	};

	// Represents offsets into a common VB/IB that represents a specific submesh of a model for drawing
	// This is essentially a "render unit" (Draw call + Pipeline states)
	struct Mesh
	{
		uint32_t numIndices;			// Vertex count to draw
		uint32_t indicesFirstIndex;		// First index in IB
		uint32_t vertexOffset;			// First index in VB
	};

	// A collection of meshes and material that represents a coherent geometric model
	struct Model
	{
	public:
		Model() = default;
		~Model() = default;

		void Initialize(const Buffer& vb, const Buffer& ib, const std::vector<std::pair<Mesh, Material>>& meshesAndMaterials);

		// Mesh have an implicit but weak relation to materials.
		// Here we ensure that we are working with them in pairs but still keeping them separate.
		const std::vector<Mesh>& GetMeshes() const;
		const std::vector<Material>& GetMaterials() const;

		ID3D11Buffer* GetVB() const;
		ID3D11Buffer* GetIB() const;

	private:
		void AddMesh(const Mesh& mesh, const Material& material);

	private:
		Buffer m_vb;
		Buffer m_ib;

		std::vector<Mesh> m_meshes;
		std::vector<Material> m_materials;
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

