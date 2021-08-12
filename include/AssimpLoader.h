#pragma once
#include <string>
#include <optional>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

struct aiMesh;
struct aiScene;
struct aiNode;
struct aiScene;

namespace Gino
{
	struct AssimpMeshSubset
	{
		unsigned int vertexStart;
		unsigned int indexStart;
		unsigned int indexCount;

		std::optional<std::string> diffuseFilePath;
		std::optional<std::string> specularFilePath;
		std::optional<std::string> normalFilePath;
		std::optional<std::string> opacityFilePath;
	};

	struct AssimpVertex
	{
		aiVector3D position;
		aiVector3D normal;
		aiVector2D uv;
		aiVector3D tangent;
		aiVector3D bitangent;

	};

	struct AssimpMaterialPaths
	{
		std::optional<std::string> diffuseFilePath;
		std::optional<std::string> specularFilePath;
		std::optional<std::string> normalFilePath;
		std::optional<std::string> opacityFilePath;
	};

	class AssimpLoader
	{
	public:
		AssimpLoader() = delete;
		AssimpLoader(const std::filesystem::path& filePath);
		~AssimpLoader() = default;

		const std::vector<AssimpVertex>& GetVertices() const;
		const std::vector<uint32_t>& GetIndices() const;
		const std::vector<AssimpMeshSubset>& GetSubsets() const;
		const std::vector<AssimpMaterialPaths> GetMaterials() const;

	private:
		void ProcessMesh(aiMesh* mesh, const aiScene* scene);
		void ProcessNode(aiNode* node, const aiScene* scene);

	private:
		uint32_t m_meshVertexCount = 0;
		uint32_t m_meshIndexCount = 0;

		std::vector<AssimpVertex> m_vertices;
		std::vector<uint32_t> m_indices;
		std::vector<AssimpMeshSubset> m_subsets;
		std::vector<AssimpMaterialPaths> m_materials;

	};

}
