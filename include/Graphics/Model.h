#pragma once
#include "ResourceTypes.h"
#include "Graphics/Material.h"
#include "Component.h"

namespace Gino
{
	// Represents offsets into a common VB/IB that represents a specific submesh of a model for drawing
	// This is essentially a "render unit" (Draw call + Pipeline states)
	struct Mesh
	{
		uint32_t numIndices;			// Vertex count to draw
		uint32_t indicesFirstIndex;		// First index in IB
		uint32_t vertexOffset;			// First index in VB
	};

	// A collection of meshes and material that represents a coherent geometric model
	class Model : public Component
	{
	public:
		Model();
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
}


