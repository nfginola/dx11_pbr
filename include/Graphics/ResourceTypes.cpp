#include "pch.h"
#include "ResourceTypes.h"

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

}