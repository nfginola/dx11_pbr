#include "pch.h"
#include "Graphics/Renderer.h"

#include "FPCamera.h"
#include "Graphics/Model.h"

// Temp
#include "Timer.h"

#include "Graphics/ImGuiRenderer.h"
#include "Graphics/SkyboxRenderer.h"

#define MAX_INSTANCES 3500

namespace Gino
{
	Renderer::Renderer(DXDevice* dxDev, bool vsync) :
		m_mainCamera(nullptr),
		m_vsync(vsync),
		m_dxDev(dxDev),
		m_imGui(std::make_unique<ImGuiRenderer>(dxDev->GetHWND(), dxDev->GetDevice(), dxDev->GetContext())),
		m_skybox(std::make_unique<SkyboxRenderer>(dxDev))
	{
		std::cout << "vsync: " << (vsync ? "on" : "off") << '\n';

		// make swapchain framebuffer
		m_finalFramebuffer.Initialize({ m_dxDev->GetBackbufferTarget() });

		auto dev = dxDev->GetDevice();
		auto ctx = dxDev->GetContext();

		// setup instancing buffer
		D3D11_BUFFER_DESC instanceDesc
		{
			.ByteWidth = sizeof(DirectX::SimpleMath::Matrix) * MAX_INSTANCES,		// Support MAX_INSTANCES in one go
			.Usage = D3D11_USAGE_DYNAMIC,
			.BindFlags = D3D11_BIND_VERTEX_BUFFER,
			.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE
		};
		m_instanceBuffer.Initialize(dev, instanceDesc);

		// setup default forward shaders with instancing layout
		m_forwardOpaquePBRShaders
			.AddStage(ShaderStage::Vertex, "compiled_shaders/ForwardPBR_VS.cso")
			.AddStage(ShaderStage::Pixel, "compiled_shaders/ForwardPBR_PS.cso")
			.AddInputDescs(Vertex_POS_UV_NORMAL::GetElementDescriptors())

			// Setup instancing data (Buffer 1)
			.AddInputDesc({ "INSTANCE_WM_ROW", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1})
			.AddInputDesc({ "INSTANCE_WM_ROW", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 })
			.AddInputDesc({ "INSTANCE_WM_ROW", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 })
			.AddInputDesc({ "INSTANCE_WM_ROW", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 })
			.Build(dev);

		m_forwardOpaquePhongShaders
			.AddStage(ShaderStage::Vertex, "compiled_shaders/ForwardPhong_VS.cso")
			.AddStage(ShaderStage::Pixel, "compiled_shaders/ForwardPhong_PS.cso")
			.AddInputDescs(Vertex_POS_UV_NORMAL::GetElementDescriptors())

			// Setup instancing data (Buffer 1)
			.AddInputDesc({ "INSTANCE_WM_ROW", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 })
			.AddInputDesc({ "INSTANCE_WM_ROW", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 })
			.AddInputDesc({ "INSTANCE_WM_ROW", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 })
			.AddInputDesc({ "INSTANCE_WM_ROW", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 })
			.Build(dev);

		D3D11_RASTERIZER_DESC1 rsD{ .FillMode = D3D11_FILL_SOLID, .CullMode = D3D11_CULL_BACK };
		HRCHECK(dev->CreateRasterizerState1(&rsD, m_rs.GetAddressOf()));
	
		// make sampler
		D3D11_SAMPLER_DESC samplerDesc
		{
			//.Filter = D3D11_FILTER_ANISOTROPIC,
			.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
			.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
			.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
			.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
			.MipLODBias = 0.f,
			.MaxAnisotropy = 8,									// We have to check max anisotropy available, but lets set to 8
			.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL,		// Not used.. what do we do with the information on whether it has passed?? // Read more
			.BorderColor = { 0.f, 0.f, 0.f, 1.f },
			.MinLOD = 0.f,
			.MaxLOD = D3D11_FLOAT32_MAX
		};
		HRCHECK(dev->CreateSamplerState(&samplerDesc, m_mainSampler.GetAddressOf()));

		// make depth buffer
		D3D11_TEXTURE2D_DESC depthDesc
		{
			.Width = (uint32_t)m_dxDev->GetBackbufferViewport().Width,
			.Height = (uint32_t)m_dxDev->GetBackbufferViewport().Height,
			.MipLevels = 1,
			.ArraySize = 1,
			.Format = DXGI_FORMAT_D32_FLOAT,		// NO STENCIL, otherwise use DXGI_FORMAT_D24_UNORM_S8_UINT
			.SampleDesc = {.Count = 1, .Quality = 0 },
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_DEPTH_STENCIL,
			.CPUAccessFlags = 0,
			.MiscFlags = 0
		};
		m_depth.Initialize(dev, ctx, depthDesc);

		// make depth stencil state (closely tied to the depth stencil view, essentially configs for writing to the DSV)
		D3D11_DEPTH_STENCIL_DESC dssDesc
		{
			.DepthEnable = true,
			.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
			.DepthFunc = D3D11_COMPARISON_LESS_EQUAL,
			.StencilEnable = false
		};
		HRCHECK(dev->CreateDepthStencilState(&dssDesc, m_dss.GetAddressOf()));
		

		m_cbPerFrame.Initialize(dev);
		m_cbPerObject.Initialize(dev);


		// Setup HDR render to texture
		D3D11_TEXTURE2D_DESC renderTexDesc
		{
			.Width = (uint32_t)m_dxDev->GetBackbufferViewport().Width,
			.Height = (uint32_t)m_dxDev->GetBackbufferViewport().Height,
			.MipLevels = 1,
			.ArraySize = 1,
			.Format = DXGI_FORMAT_R16G16B16A16_FLOAT,		// Floating point 16bit for HDR
			.SampleDesc = {.Count = 1, .Quality = 0 },
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
			.CPUAccessFlags = 0,
			.MiscFlags = 0
		};
		m_renderTexture.Initialize(dev, ctx, renderTexDesc);

		m_renderFramebuffer.Initialize({ m_renderTexture.GetRTV() }, m_depth.GetDSV());

		m_fullscreenQuadShaders
			.AddStage(ShaderStage::Vertex, "compiled_shaders/quadpass_vs.cso")
			.AddStage(ShaderStage::Pixel, "compiled_shaders/quadpass_ps.cso")
			.Build(dev);

		// make point sampler
		D3D11_SAMPLER_DESC pointSSDesc
		{
			.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
			.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
			.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
			.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
			.MipLODBias = 0.f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL,
			.BorderColor = { 0.f, 0.f, 0.f, 1.f },
			.MinLOD = 0.f,
			.MaxLOD = 0.f
		};
		HRCHECK(dev->CreateSamplerState(&pointSSDesc, m_pointSampler.GetAddressOf()));

		// setup point light structued buffer
		m_sbPointLights.Initialize(dev, StructuredBufferDesc<SB_PointLight>{.elementCount = 6, .dynamic = true, .cpuWrite = true});



	}

	Renderer::~Renderer()
	{
	}

	void Renderer::SetRenderCamera(FPCamera* cam)
	{
		m_mainCamera = cam;

		if (m_skybox)
		{
			m_skybox->SetCamera(cam);
		}
	}

	void Renderer::SetModels(const std::vector<std::pair<Model*, std::vector<Transform*>>>* models)
	{
		m_opaqueModels = models;
	}
	
	void Renderer::BeginFrame()
	{
		m_imGui->BeginFrame();
	}

	void Renderer::EndFrame()
	{
		auto ctx = m_dxDev->GetContext();

		// Draw UI directly on swapchain backbuffer (no post-process or anything applied to ImGUI)
		m_imGui->EndFrame(ctx, m_finalFramebuffer);

		// Present to swapchain
		m_dxDev->GetSwapChain()->Present(m_vsync ? 1 : 0, 0);
	}

	static bool norMapOn = true;
	static bool aoTexOn = true;
	void Renderer::Render()
	{
		assert(m_mainCamera != nullptr);
		auto ctx = m_dxDev->GetContext();

		ImGui::Begin("PBR Renderer Settings");
		ImGui::Checkbox("Normal Mapping", &norMapOn);
		ImGui::Checkbox("AO Texture", &aoTexOn);
		ImGui::End();

		// Update frame data for GPU
		{
			// Update per frame cb
			m_cbPerFrame.data.view = m_mainCamera->GetViewMatrix();
			m_cbPerFrame.data.projection = m_mainCamera->GetProjectionMatrix();
			m_cbPerFrame.data.cameraPosition = m_mainCamera->GetPosition();
			m_cbPerFrame.data.normalMapOn = norMapOn ? 1.f : 0.f;
			m_cbPerFrame.data.aoTexOn = aoTexOn ? 1.f : 0.f;
			m_cbPerFrame.Upload(ctx);

			// Update light list
			D3D11_MAPPED_SUBRESOURCE plMapped;
			ctx->Map(m_sbPointLights.buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &plMapped);
			auto light = (SB_PointLight*)plMapped.pData;
			light[0] = { .position = {30.f, 50.f, 30.f, 1.f }, .color = { 1200.f, 1200.f, 1200.f, 1.f } };

			// whites
			//light[1] = { .position = {30.f, 50.f, -30.f, 1.f }, .color = { 1200.f, 1200.f, 1200.f, 1.f } };
			//light[2] = { .position = {-30.f, 50.f, 30.f, 1.f }, .color = { 1200.f, 1200.f, 1200.f, 1.f } };
			//light[3] = { .position = {-30.f, 50.f, -30.f, 1.f }, .color = { 1200.f, 1200.f, 1200.f, 1.f } };
			//light[4] = { .position = {80.f, 50.f, 0.f, 1.f }, .color = { 1200.f, 1200.f, 1200.f, 1.f } };
			//light[5] = { .position = {-80.f, 50.f, 0.f, 1.f }, .color = { 1200.f, 1200.f, 1200.f, 1.f } };
			// non whites
			light[1] = { .position = {30.f, 50.f, -30.f, 1.f }, .color = { 0.f, 1200.f, 0.f, 1.f } };
			light[2] = { .position = {-30.f, 50.f, 30.f, 1.f }, .color = { 0.f, 0.f, 1200.f, 1.f } };
			light[3] = { .position = {-30.f, 50.f, -30.f, 1.f }, .color = { 1200.f, 0.f, 1200.f, 1.f } };
			light[4] = { .position = {80.f, 50.f, 0.f, 1.f }, .color = { 0.f, 1200.f, 1200.f, 1.f } };
			light[5] = { .position = {-80.f, 50.f, 0.f, 1.f }, .color = { 1200.f, 0.f, 0.f, 1.f } };
			ctx->Unmap(m_sbPointLights.buffer.Get(), 0);
		}

		// Clear main render texture target
		m_renderFramebuffer.Clear(ctx, { { 0.529f, 0.808f, 0.922f, 1.f } });

		// Render skybox
		m_skybox->Render(m_renderFramebuffer, m_dxDev->GetBackbufferViewport());

		// Render models
		Timer opaquePassTimer;
		{
			// Set state for render to texture (models)
			ctx->VSSetConstantBuffers(0, 1, m_cbPerFrame.buffer.GetAddressOf());
			ctx->PSSetConstantBuffers(0, 1, m_cbPerFrame.buffer.GetAddressOf());
			// set rasterizer state
			D3D11_VIEWPORT viewports[] = { m_dxDev->GetBackbufferViewport() };
			ctx->RSSetViewports(_countof(viewports), viewports);
			ctx->RSSetState(m_rs.Get());

			ctx->PSSetShaderResources(7, 1, m_sbPointLights.srv.GetAddressOf());

			ID3D11SamplerState* samplers[] = { m_mainSampler.Get(), m_pointSampler.Get() };
			ctx->PSSetSamplers(0, _countof(samplers), samplers);

			// Render to texture
			m_renderFramebuffer.Bind(ctx);
			ctx->OMSetDepthStencilState(m_dss.Get(), 0);

			// Render models
			for (const auto& modelInstance : *m_opaqueModels)
			{
				const auto& model = modelInstance.first;
				const auto& instances = modelInstance.second;

				// We guarantee that the material type for a whole model is identical
				auto matType = model->GetMaterials()[0].GetType();
				if (matType == MaterialType::PBR)
				{
					m_forwardOpaquePBRShaders.Bind(ctx);
				}
				else if (matType == MaterialType::Phong)
				{
					m_forwardOpaquePhongShaders.Bind(ctx);
				}


				assert(instances.size() <= MAX_INSTANCES);

				// Fill instance data
				D3D11_MAPPED_SUBRESOURCE mappedInstSubres;
				ctx->Map(m_instanceBuffer.buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedInstSubres);
				auto seat = (DirectX::SimpleMath::Matrix*)mappedInstSubres.pData;
				for (int i = 0; i < instances.size(); ++i)
				{
					seat[i] = instances[i]->GetWorldMatrix();
				}
				ctx->Unmap(m_instanceBuffer.buffer.Get(), 0);

				ID3D11Buffer* vbs[] = { model->GetVB(), m_instanceBuffer.buffer.Get() };
				UINT vbStrides[] = { sizeof(Vertex_POS_UV_NORMAL), sizeof(DirectX::SimpleMath::Matrix) };
				UINT vbOffsets[] = { 0, 0 };
				ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				ctx->IASetVertexBuffers(0, _countof(vbs), vbs, vbStrides, vbOffsets);
				ctx->IASetIndexBuffer(model->GetIB(), DXGI_FORMAT_R32_UINT, 0);
				ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				const auto& meshes = model->GetMeshes();
				const auto& materials = model->GetMaterials();
				assert(meshes.size() == materials.size());

				// Draw submeshes
				for (uint32_t i = 0; i < meshes.size(); ++i)
				{
					// Bind material (PBR)
					if (matType == MaterialType::PBR)
					{
						ID3D11ShaderResourceView* srvs[] =
						{
							materials[i].GetProperties<PBRMaterialData>().albedo->GetSRV() ,
							materials[i].GetProperties<PBRMaterialData>().metallicAndRoughness->GetSRV(),
							materials[i].GetProperties<PBRMaterialData>().normal->GetSRV(),
							materials[i].GetProperties<PBRMaterialData>().ao->GetSRV(),
							materials[i].GetProperties<PBRMaterialData>().emission->GetSRV()
						};

						ctx->PSSetShaderResources(0, _countof(srvs), srvs);
					}
					else if (matType == MaterialType::Phong)
					{
						ID3D11ShaderResourceView* srvs[] =
						{
							materials[i].GetProperties<PhongMaterialData>().diffuse->GetSRV(),
							materials[i].GetProperties<PhongMaterialData>().specular->GetSRV(),
							materials[i].GetProperties<PhongMaterialData>().normal->GetSRV(),
							materials[i].GetProperties<PhongMaterialData>().opacity->GetSRV()
						};

						ctx->PSSetShaderResources(0, _countof(srvs), srvs);
					}

					ctx->DrawIndexedInstanced(meshes[i].numIndices, (uint32_t)instances.size(), meshes[i].indicesFirstIndex, meshes[i].vertexOffset, 0);

					//// no instancing
					//for (int instanceID = 0; instanceID < modelInstance.second.size(); ++instanceID)
					//{
					//	m_cbPerObject.data.model = modelInstance.second[instanceID]->GetWorldMatrix();
					//	m_cbPerObject.Upload(ctx);
					//	ctx->VSSetConstantBuffers(1, 1, m_cbPerObject.buffer.GetAddressOf());

					//	ctx->DrawIndexedInstanced(meshes[i].numIndices, 1, meshes[i].indicesFirstIndex, meshes[i].vertexOffset, 0);
					//}
				}
			}

			// Unbind framebuffer so that we can read textures associated with it
			m_renderFramebuffer.Unbind(ctx);
		}
		ImGui::Begin("Frame Statistics");
		ImGui::Text("Opaque Draw Pass CPU %s ms", std::to_string(opaquePassTimer.TimeElapsed() * 1000.f).c_str());
		ImGui::End();

		// Render fullscreen quad pass
		Timer quadPassTimer;
		{
			m_fullscreenQuadShaders.Bind(ctx);
			m_finalFramebuffer.Clear(ctx);
			m_finalFramebuffer.Bind(ctx);

			// Bind previous framebuffer render content for reading
			ID3D11ShaderResourceView* srvs[] = { m_renderTexture.GetSRV() };
			ctx->PSSetShaderResources(0, _countof(srvs), srvs);

			// Point sample
			ID3D11SamplerState* smplrs[] = { m_pointSampler.Get() };
			ctx->PSSetSamplers(0, _countof(smplrs), smplrs);

			// Draw quad
			ctx->Draw(6, 0);

			// Unbind renderTexture from read-bind so that it can be reused for writing in the next frame
			ID3D11ShaderResourceView* nullSRVs[] = { nullptr };
			ctx->PSSetShaderResources(0, 1, nullSRVs);

		}
		ImGui::Begin("Frame Statistics");
		ImGui::Text("Quad Pass CPU %s ms", std::to_string(quadPassTimer.TimeElapsed() * 1000.f).c_str());
		ImGui::End();

	}

	ImGuiRenderer* Renderer::GetImGui() const
	{
		return m_imGui.get();
	}

}

