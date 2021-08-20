#pragma once
#include <string>
#include <optional>
#include <variant>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <assimp/pbrmaterial.h>


struct aiMesh;
struct aiScene;
struct aiNode;
struct aiScene;

namespace Gino
{
	struct AssimpMaterialPaths
	{
		std::optional<std::string> diffuseFilePath;
		std::optional<std::string> specularFilePath;
		std::optional<std::string> normalFilePath;
		std::optional<std::string> opacityFilePath;
	};

	struct AssimpMaterialPathsPBR
	{
		// Ref: glTF overview https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/figures/gltfOverview-2.0.0b.png
		std::optional<std::string> albedo;
		std::optional<std::string> normal;
		std::optional<std::string> metallicAndRoughness;		// Metalness in BLUE channel, roughness in GREEN channel
		std::optional<std::string> ao;
		std::optional<std::string> emission;


		aiVector3D baseColorFactor;				// Multiplied component wise with albedo
		aiVector2D metallicAndRoughnessFactor;	// Multiplied component wise with metallic and roughness

		/*
		implement later
		* 
		normalTextureFactor (scale)
		occlusionFactor (ao strength)
		emissiveFactor (scaling factor for emissive texture)
		*/


	};


	struct AssimpMeshSubset
	{
		unsigned int vertexStart;
		unsigned int indexStart;
		unsigned int indexCount;
		std::variant<AssimpMaterialPaths, AssimpMaterialPathsPBR> mats;
	};


	struct AssimpVertex
	{
		aiVector3D position;
		aiVector3D normal;
		aiVector2D uv;
		aiVector3D tangent;
		aiVector3D bitangent;

	};


	/*
	
	To do:

	- Refactor material grabbing so that the duplicates are removed (Grab materials in ctor and grab materials in ProcessMesh..)
	-- Priority: Low
	
	*/

	class AssimpLoader
	{
	public:
		AssimpLoader() = delete;
		AssimpLoader(const std::filesystem::path& filePath, bool PBR = false);
		~AssimpLoader() = default;

		const std::vector<AssimpVertex>& GetVertices() const;
		const std::vector<uint32_t>& GetIndices() const;

		const std::vector<AssimpMeshSubset>& GetSubsets() const;
		const std::vector<AssimpMaterialPaths>& GetMaterials() const;

		const std::vector<AssimpMaterialPathsPBR>& GetMaterialsPBR() const;


	private:
		void ProcessMesh(aiMesh* mesh, const aiScene* scene);
		void ProcessNode(aiNode* node, const aiScene* scene);

	private:
		bool m_PBR;
		std::filesystem::path m_filePath;

		uint32_t m_meshVertexCount = 0;
		uint32_t m_meshIndexCount = 0;

		std::vector<AssimpVertex> m_vertices;
		std::vector<uint32_t> m_indices;
		std::vector<AssimpMeshSubset> m_subsets;
		std::vector<AssimpMaterialPaths> m_materials;

		std::vector<AssimpMaterialPathsPBR> m_materialsPBR;

	};

}
