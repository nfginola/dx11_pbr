#include "pch.h"
#include "Graphics/ShaderGroup.h"

namespace Gino
{
	ShaderGroup::ShaderGroup()
	{
	}

	ShaderGroup::~ShaderGroup()
	{
	}

	ShaderGroup& ShaderGroup::AddStage(ShaderStage stage, const std::vector<uint8_t>& code)
	{
		ShaderModule mod
		{
			.stage = stage,
			.code = code
		};

		switch (stage)
		{
		case ShaderStage::Vertex:
			mod.createFunc = [this](const DevicePtr& dev, const std::vector<uint8_t>& code) { dev->CreateVertexShader(code.data(), code.size(), nullptr, this->m_vs.GetAddressOf()); };
			break;
		case ShaderStage::Hull:
			mod.createFunc = [this](const DevicePtr& dev, const std::vector<uint8_t>& code) { dev->CreateHullShader(code.data(), code.size(), nullptr, this->m_hs.GetAddressOf()); };
			break;
		case ShaderStage::Domain:
			mod.createFunc = [this](const DevicePtr& dev, const std::vector<uint8_t>& code) { dev->CreateDomainShader(code.data(), code.size(), nullptr, this->m_ds.GetAddressOf()); };
			break;
		case ShaderStage::Geometry:
			mod.createFunc = [this](const DevicePtr& dev, const std::vector<uint8_t>& code) { dev->CreateGeometryShader(code.data(), code.size(), nullptr, this->m_gs.GetAddressOf()); };
			break;	
		case ShaderStage::Pixel:
			mod.createFunc = [this](const DevicePtr& dev, const std::vector<uint8_t>& code) { dev->CreatePixelShader(code.data(), code.size(), nullptr, this->m_ps.GetAddressOf()); };
			break;
		case ShaderStage::Compute:
			mod.createFunc = [this](const DevicePtr& dev, const std::vector<uint8_t>& code) { dev->CreateComputeShader(code.data(), code.size(), nullptr, this->m_cs.GetAddressOf()); };
			break;
		default:
			assert(false);
		}
		
		m_modules.push_back(mod);
		return *this;
	}
	ShaderGroup& ShaderGroup::AddInputDescs(const std::vector<D3D11_INPUT_ELEMENT_DESC> descs)
	{
		for (const auto& desc : descs)
		{
			m_inputDescs.push_back(desc);
		}
		return *this;
	}

	ShaderGroup& ShaderGroup::AddInputDesc(const D3D11_INPUT_ELEMENT_DESC& desc)
	{
		m_inputDescs.push_back(desc);
		return *this;
	}

	void ShaderGroup::Build(DevicePtr dev)
	{
		for (const auto& mod : m_modules)
		{
			mod.createFunc(dev, mod.code);

			if (mod.stage == ShaderStage::Vertex)
			{
				assert(!m_inputDescs.empty());
				dev->CreateInputLayout(m_inputDescs.data(), m_inputDescs.size(), mod.code.data(), mod.code.size(), m_inputLayout.GetAddressOf());
			}
		}
	}
	void ShaderGroup::Bind(DeviceContextPtr ctx)
	{
		if (m_vs)
		{
			ctx->IASetInputLayout(m_inputLayout.Get());
			ctx->VSSetShader(m_vs.Get(), nullptr, 0);
		}
		if (m_hs)
		{
			ctx->HSSetShader(m_hs.Get(), nullptr, 0);
		}
		if (m_ds)
		{
			ctx->DSSetShader(m_ds.Get(), nullptr, 0);
		}
		if (m_gs)
		{
			ctx->GSSetShader(m_gs.Get(), nullptr, 0);
		}
		if (m_ps)
		{
			ctx->PSSetShader(m_ps.Get(), nullptr, 0);
		}
		if (m_cs)
		{
			ctx->CSSetShader(m_cs.Get(), nullptr, 0);
		}
	}
}

