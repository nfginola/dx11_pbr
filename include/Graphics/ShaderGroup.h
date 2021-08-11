#pragma once
#include "DXDevice.h"
#include <array>
#include <functional>

namespace Gino
{
	enum class ShaderStage
	{
		Vertex,
		Hull,
		Domain,
		Geometry,
		Pixel,
		Compute
	};

	struct ShaderModule
	{
		ShaderStage stage;
		std::vector<uint8_t> code;
		std::function<void(const DevicePtr&, const std::vector<uint8_t>&)> createFunc;
	};

	class ShaderGroup
	{
	public:
		ShaderGroup();
		~ShaderGroup();

		ShaderGroup& AddStage(ShaderStage stage, const std::filesystem::path& filePath);
		ShaderGroup& AddInputDescs(const std::vector<D3D11_INPUT_ELEMENT_DESC> descs);
		ShaderGroup& AddInputDesc(const D3D11_INPUT_ELEMENT_DESC& desc);
		void Build(DevicePtr dev);
		void Bind(DeviceContextPtr ctx);

	private:
		VsPtr m_vs;
		InputLayoutPtr m_inputLayout;
		std::vector<D3D11_INPUT_ELEMENT_DESC> m_inputDescs;

		HsPtr m_hs;
		DsPtr m_ds;
		GsPtr m_gs;
		PsPtr m_ps;
		CsPtr m_cs;

		std::vector<ShaderModule> m_modules;
	};
}
