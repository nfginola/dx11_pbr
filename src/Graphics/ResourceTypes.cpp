#include "pch.h"
#include "Graphics/ResourceTypes.h"

namespace Gino
{
    std::vector<D3D11_INPUT_ELEMENT_DESC> Vertex_POS_UV_NORMAL::GetElementDescriptors()
    {
        std::vector<D3D11_INPUT_ELEMENT_DESC> descriptor =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        return descriptor;
    }

    void Buffer::Initialize(const DevicePtr& dev, const D3D11_BUFFER_DESC& desc, const void* initData)
    {
        D3D11_SUBRESOURCE_DATA subres
        {
            .pSysMem = initData,
            .SysMemPitch = desc.ByteWidth
        };
        HRCHECK(dev->CreateBuffer(&desc, &subres, buffer.GetAddressOf()));
    }

    void Buffer::Initialize(const DevicePtr& dev, VertexBufferDescRaw desc)
    {
        D3D11_BUFFER_DESC vbDesc
        {
            .ByteWidth = static_cast<uint32_t>(desc.totalSize),         // narrowing cast from size_t to unsigned
            .Usage = D3D11_USAGE_IMMUTABLE,
            .BindFlags = D3D11_BIND_VERTEX_BUFFER
        };
        D3D11_SUBRESOURCE_DATA vbDat
        {
            .pSysMem = desc.data,
            .SysMemPitch = vbDesc.ByteWidth
        };
        HRCHECK(dev->CreateBuffer(&vbDesc, &vbDat, buffer.GetAddressOf()));
    }

    void Buffer::Initialize(const DevicePtr& dev, IndexBufferDesc desc)
    {
        D3D11_BUFFER_DESC ibDesc
        {
            .ByteWidth = static_cast<uint32_t>(desc.data.size()) * sizeof(uint32_t),
            .Usage = D3D11_USAGE_IMMUTABLE,
            .BindFlags = D3D11_BIND_INDEX_BUFFER,
        };
        D3D11_SUBRESOURCE_DATA ibDat
        {
            .pSysMem = desc.data.data(),
            .SysMemPitch = ibDesc.ByteWidth
        };
        HRCHECK(dev->CreateBuffer(&ibDesc, &ibDat, buffer.GetAddressOf()));
    }

    void Buffer::Initialize(const DevicePtr& dev, StructuredBufferDesc desc)
    {
        std::cout << "Nagi::Buffer : Not implemented";
        assert(false);      // To extend
    }

    void Buffer::Initialize(const DevicePtr& dev, RWBufferDesc desc)
    {
        std::cout << "Nagi::Buffer : Not implemented";
        assert(false);      // To extend
    }

    void Buffer::Initialize(const DevicePtr& dev, ByteAddressBufferDesc desc)
    {
        std::cout << "Nagi::Buffer : Not implemented";
        assert(false);      // To extend
    }

    void Buffer::Initialize(const DevicePtr& dev, IndirectArgsBufferDesc desc)
    {
        std::cout << "Nagi::Buffer : Not implemented";
        assert(false);      // To extend
    }


    void Framebuffer::Initialize(std::array<RtvPtr, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> targets, DsvPtr dsv)
    {
        m_depthStencilView = dsv;

        for (uint32_t i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        {
            if (targets[i])
            {
                m_renderTargets[i] = targets[i].Get(); 
                ++m_activeRenderTargets;
                // Internal increment will happen here which is not tracked by the smart pointer
                // CORRECTION: actually, there is no increment happening internally! Great!
            }
            else
            {
                // No gaps allowed
                break;
            }
        }
    }

    void Framebuffer::Clear(const DeviceContextPtr& ctx, const std::array<FLOAT[4], D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> clearValues, const DepthStencilClearDesc& dsClearDesc)
    {
        for (uint32_t i = 0; i < m_activeRenderTargets; ++i)
        {
            ctx->ClearRenderTargetView(m_renderTargets[i], clearValues[i]);
            if (m_depthStencilView)
            {
                ctx->ClearDepthStencilView(m_depthStencilView.Get(), dsClearDesc.clearFlags, dsClearDesc.depth, dsClearDesc.stencil);
            }
        }
    }

    void Framebuffer::Bind(const DeviceContextPtr& ctx)
    {
        assert(m_activeRenderTargets > 0);
        if (m_depthStencilView)
        {
            ctx->OMSetRenderTargets(m_activeRenderTargets, m_renderTargets.data(), m_depthStencilView.Get());
        }
        else
        {
            ctx->OMSetRenderTargets(m_activeRenderTargets, m_renderTargets.data(), nullptr);
        }
    }

    Framebuffer::Framebuffer() :
        m_renderTargets({}),
        m_activeRenderTargets(0),
        m_depthStencilView(nullptr)
    {
    }

    Framebuffer::~Framebuffer()
    {
        //for (uint32_t i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        //{
        //    if (m_renderTargets[i])
        //    {
        //        m_renderTargets[i]->Release();
        //    }
        //    else
        //    {
        //        // No gaps allowed
        //        break;
        //    }
        //}
    }

    const std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> Framebuffer::GetRenderTargets() const
    {
        return m_renderTargets;
    }

    const DsvPtr& Framebuffer::GetDepthStencilView() const
    {
        return m_depthStencilView;
    }

    ID3D11Texture2D* Texture::GetTexture() const
    {
        return m_texture.Get();
    }

    void Texture::Initialize(const DevicePtr& dev, const DeviceContextPtr& ctx, const D3D11_TEXTURE2D_DESC& desc, Utils::ImageData* imageData)
    {
        // Immutable not allowed for Textures
        // We want to make use of auto mipmap generation which requires GPU RW access
        assert( (desc.Usage & D3D11_USAGE_IMMUTABLE) == 0);     

        if (imageData)
        {
            HRCHECK(dev->CreateTexture2D(&desc, nullptr, m_texture.GetAddressOf()));

            // Fill in data
            assert((desc.Usage & D3D11_USAGE_DEFAULT) == D3D11_USAGE_DEFAULT);
            unsigned int rowPitch = imageData->texWidth * sizeof(uint32_t);
            ctx->UpdateSubresource(m_texture.Get(), 0, nullptr, imageData->pixels, rowPitch, 0);

            /*
            So, why do we force USAGE_DEFAULT and fill in data this way instead of using pInitialData?
            Well, it is mainly so that we can more easily make use of the GenerateMips() functionality.

            If we set the Tex2D desc MipLevels = 0 (to automatically generate mip levels) but supply pInitialData[0] (mip level 0) with our texture,
            it instead seems like it expects us to fill the rest of pInitialData (the other mip levels).
            Meaning that it can only automatically generate mip levels with user supplied data (not making use of GenerateMips)

            To make use of generate mips, we go around this by:
            - Creating the texture with auto generate mips on (MipLevels = 0), no initial data, generate mips misc flag on, RTV and SRV bind on, GPU read/write
                --> This makes a texture, which generates mip levels accordingly to the dimensions given but are empty (no data)
            - We use UpdateSubresource to copy our image from CPU to GPU onto Subresource 0 (Mip Level 0) to "initialize" the texture
            - Then we use context->GenerateMips() to let the GPU recursively do the halved resolution copies    

            === We initialize Mip Level 0 because of this below (MSDN): ===
            xxx "GenerateMips uses the largest mipmap level of the view to recursively 
                generate the lower levels of the mip and stops with the smallest level that is specified by the view"
            */
        }
        else
        {
            HRCHECK(dev->CreateTexture2D(&desc, nullptr, m_texture.GetAddressOf()));
        }

        CreateViews(dev, ctx, desc);
    }

    ID3D11ShaderResourceView* Texture::GetSRV() const
    {
        if (m_srv)
        {
            return m_srv.Get();
        }
        else
        {
            std::cout << "Gino::Texture : No SRV exists for this Texture\n";
            assert(false);
            return nullptr;
        }
    }

    ID3D11RenderTargetView* Texture::GetRTV() const
    {
        if (m_rtv)
        {
            return m_rtv.Get();
        }
        else
        {
            std::cout << "Gino::Texture : No RTV exists for this Texture\n";
            assert(false);
            return nullptr;
        }
    }

    ID3D11DepthStencilView* Texture::GetDSV() const
    {
        if (m_dsv)
        {
            return m_dsv.Get();
        }
        else
        {
            std::cout << "Gino::Texture : No DSV exists for this Texture\n";
            assert(false);
            return nullptr;
        }
    }

    ID3D11UnorderedAccessView* Texture::GetUAV() const
    {
        if (m_uav)
        {
            return m_uav.Get();
        }
        else
        {
            std::cout << "Gino::Texture : No UAV exists for this Texture\n";
            assert(false);
            return nullptr;
        }
    }

    void Texture::InitializeFromFile(const DevicePtr& dev, const DeviceContextPtr& ctx, const std::filesystem::path& filePath, bool srgb, bool genMipMaps)
    {
        auto imageData = Utils::ReadImageFile(filePath);
        
        DXGI_FORMAT format = srgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
        UINT miscFlags = genMipMaps ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
        UINT mipLevels = genMipMaps ? 0 : 1;
        UINT bindFlags = genMipMaps ? D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET : D3D11_BIND_SHADER_RESOURCE;

        D3D11_TEXTURE2D_DESC texDesc
        {
            .Width = imageData.texWidth,
            .Height = imageData.texHeight,
            .MipLevels = mipLevels,
            .ArraySize = 1,
            .Format = format,
            .SampleDesc = {.Count = 1, .Quality = 0 },      // Hardcoded multisample settings for now
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = bindFlags,
            .CPUAccessFlags = 0,
            .MiscFlags = miscFlags
        };

        this->Initialize(dev, ctx, texDesc, &imageData);

        // Examples:

        //// With mipmap generation
        //D3D11_TEXTURE2D_DESC texDesc
        //{
        //	.Width = imageDat.texWidth,
        //	.Height = imageDat.texHeight,
        //	.MipLevels = 0,									// gen full mipmaps
        //	.ArraySize = 1,									
        //	.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,		// assume SRGB 
        //	.SampleDesc = { .Count = 1, .Quality = 0 },		// multisampling options
        //	.Usage = D3D11_USAGE_DEFAULT,					// allow gpu rw
        //	.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,		
        //	.CPUAccessFlags = 0,
        //	.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS	// tell em that we will use GenerateMips function
        //};

        //// No mipmap generation
        //D3D11_TEXTURE2D_DESC texDesc
        //{
        //    .Width = imageDat.texWidth,
        //    .Height = imageDat.texHeight,
        //    .MipLevels = 1,
        //    .ArraySize = 1,
        //    .Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        //    .SampleDesc = {.Count = 1, .Quality = 0 },
        //    .Usage = D3D11_USAGE_DEFAULT,
        //    .BindFlags = D3D11_BIND_SHADER_RESOURCE,
        //    .CPUAccessFlags = 0,
        //    .MiscFlags = 0
        //};
    }

    void Texture::CreateViews(const DevicePtr& dev, const DeviceContextPtr& ctx, const D3D11_TEXTURE2D_DESC& desc)
    {
        bool shouldCreateSRV = false;
        bool shouldCreateRTV = false;
        bool shouldCreateDSV = false;
        bool shouldCreateUAV = false;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = desc.Format;

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = desc.Format;

        if ((desc.BindFlags & D3D11_BIND_SHADER_RESOURCE) == D3D11_BIND_SHADER_RESOURCE)
        {
            shouldCreateSRV = true;
            if (desc.ArraySize == 1)
            {
                D3D11_TEX2D_SRV tex2DSrvDesc
                {
                    .MostDetailedMip = 0,
                    .MipLevels = (uint32_t)-1    // MipLevels: Set to -1 to indicate all the mipmap levels from MostDetailedMip on down to least detailed
                };
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D = tex2DSrvDesc;
            }
        }

        if ((desc.BindFlags & D3D11_BIND_DEPTH_STENCIL) == D3D11_BIND_DEPTH_STENCIL)
        {
            shouldCreateDSV = true;
            assert(desc.ArraySize == 1);

            D3D11_TEX2D_DSV tex2DDsvDesc
            {
                .MipSlice = 0
            };
            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Flags = 0;
        }

        if (shouldCreateSRV)
        {
            HRCHECK(dev->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_srv.GetAddressOf()));
            if (desc.MipLevels == 0 && (desc.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS) == D3D11_RESOURCE_MISC_GENERATE_MIPS)
            {
                ctx->GenerateMips(m_srv.Get());     // Automatically generate mips if more mip levels specified
            }
        }
        if (shouldCreateRTV)
        {

        }
        if (shouldCreateDSV)
        {
            HRCHECK(dev->CreateDepthStencilView(m_texture.Get(), &dsvDesc, m_dsv.GetAddressOf()));
        }
        if (shouldCreateUAV)
        {

        }
    }

    void Material::Initialize(const PhongMaterialData& data)
    {
        m_type = MaterialType::PHONG;
        m_data = data;
    }

    void Material::Initialize(const PBRMaterialData& data)
    {
        m_type = MaterialType::PBR;
        m_data = data;
    }

    MaterialType Material::GetType() const
    {
        return m_type;
    }

    void Model::Initialize(const Buffer& vb, const Buffer& ib, const std::vector<std::pair<Mesh, Material>>& meshesAndMaterials)
    {
        m_vb = vb;
        m_ib = ib;

        for (const auto& pair : meshesAndMaterials)
        {
            AddMesh(pair.first, pair.second);
        }

    }

    void Model::AddMesh(const Mesh& mesh, const Material& material)
    {
        m_meshes.push_back(mesh);
        m_materials.push_back(material);
        assert(m_meshes.size() == m_materials.size());
    }

    const std::vector<Mesh>& Model::GetMeshes() const
    {
        return m_meshes;
    }

    const std::vector<Material>& Model::GetMaterials() const
    {
        return m_materials;
    }

    ID3D11Buffer* Model::GetVB() const
    {
        return m_vb.buffer.Get();
    }

    ID3D11Buffer* Model::GetIB() const
    {
        return m_ib.buffer.Get();
    }



}