#include "pch.h"
#include "AssimpLoader.h"


namespace Gino
{
	AssimpLoader::AssimpLoader(const std::filesystem::path& filePath, bool PBR) :
		m_filePath(filePath),
		m_PBR(PBR)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			filePath.relative_path().string().c_str(),
			aiProcess_Triangulate |
			
			// For Direct3D
			aiProcess_ConvertToLeftHanded |
			//aiProcess_FlipUVs |					// (0, 0) is top left
			//aiProcess_FlipWindingOrder |		// D3D front face is CW

			aiProcess_GenSmoothNormals |
			aiProcess_CalcTangentSpace
		);

		// === WARNING - There are two place that use this directory
		// One is here in Grab materials, but there is a duplicate in ProcessMesh!!!
		const std::string directory = filePath.parent_path().string() + "/";

		if (scene == nullptr)
		{
			std::cout << "Gino::AssimpLoader : File not found!\n";
			assert(false);
		}

		// Pre-allocate memory for resources
		unsigned int totalVertexCount = 0;
		unsigned int totalSubsetCount = 0;
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
		{
			totalVertexCount += scene->mMeshes[i]->mNumVertices;
		}

		m_vertices.reserve(totalVertexCount);
		m_indices.reserve(totalVertexCount);
		m_subsets.reserve(scene->mNumMeshes);

		// Grab materials

		if (!PBR)
		{
			m_materials.reserve(scene->mNumMaterials);
			for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
			{
				auto mtl = scene->mMaterials[i];
				aiString diffPath, norPath, opacityPath, specularPath, bruh;
				mtl->GetTexture(aiTextureType_DIFFUSE, 0, &diffPath);

				aiReturn norRet = mtl->GetTexture(aiTextureType_NORMALS, 0, &norPath);
				if (norRet != aiReturn_SUCCESS)
					mtl->GetTexture(aiTextureType_HEIGHT, 0, &norPath);

				mtl->GetTexture(aiTextureType_OPACITY, 0, &opacityPath);
				mtl->GetTexture(aiTextureType_SPECULAR, 0, &specularPath);
				mtl->GetTexture(aiTextureType_UNKNOWN, 0, &bruh);


				AssimpMaterialPaths matPaths;
				matPaths.diffuseFilePath = (diffPath.length == 0) ? std::nullopt : std::optional<std::string>(std::string(directory + diffPath.C_Str()));
				matPaths.normalFilePath = (norPath.length == 0) ? std::nullopt : std::optional<std::string>(std::string(directory + norPath.C_Str()));
				matPaths.opacityFilePath = (opacityPath.length == 0) ? std::nullopt : std::optional<std::string>(std::string(directory + opacityPath.C_Str()));
				matPaths.specularFilePath = (specularPath.length == 0) ? std::nullopt : std::optional<std::string>(std::string(directory + specularPath.C_Str()));
				m_materials.push_back(matPaths);
			}
		}
		else
		{
			// PBR
			m_materialsPBR.reserve(scene->mNumMaterials);
			for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
			{
				auto mtl = scene->mMaterials[i];
				aiString albedoPath, norPath, metallicPath, roughnessPath, aoPath, emissionPath;

				mtl->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &albedoPath);

				aiReturn norRet = mtl->GetTexture(aiTextureType_NORMALS, 0, &norPath);
				if (norRet != aiReturn_SUCCESS)
					mtl->GetTexture(aiTextureType_HEIGHT, 0, &norPath);

				mtl->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &metallicPath);
				mtl->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &roughnessPath);
				mtl->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &aoPath);
				mtl->GetTexture(aiTextureType_EMISSIVE, 0, &emissionPath);


				//aiString paths[aiTextureType_UNKNOWN];
				//for (int x = 0; x < aiTextureType_UNKNOWN; ++x)
				//{
				//	mtl->GetTexture((aiTextureType)(x + 1), 0, &paths[x]);
				//}


				AssimpMaterialPathsPBR matPaths;
				matPaths.albedo = (albedoPath.length == 0) ? std::nullopt : std::optional<std::string>(std::string(directory + albedoPath.C_Str()));
				matPaths.normal = (norPath.length == 0) ? std::nullopt : std::optional<std::string>(std::string(directory + norPath.C_Str()));
				matPaths.metallicAndRoughness = (metallicPath.length == 0) ? std::nullopt : std::optional<std::string>(std::string(directory + metallicPath.C_Str()));
				matPaths.ao = (aoPath.length == 0) ? std::nullopt : std::optional<std::string>(std::string(directory + aoPath.C_Str()));
				matPaths.emission = (emissionPath.length == 0) ? std::nullopt : std::optional<std::string>(std::string(directory + emissionPath.C_Str()));
				
				mtl->Get("$mat.gltf.pbrMetallicRoughness.baseColorFactor", 0, 0, matPaths.baseColorFactor.x);
				mtl->Get("$mat.gltf.pbrMetallicRoughness.baseColorFactor", 0, 1, matPaths.baseColorFactor.y);
				mtl->Get("$mat.gltf.pbrMetallicRoughness.baseColorFactor", 0, 2, matPaths.baseColorFactor.z);

				mtl->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, matPaths.metallicAndRoughnessFactor.x);
				mtl->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, matPaths.metallicAndRoughnessFactor.y);

				m_materialsPBR.push_back(matPaths);
			}
		}

		



		// Start processing
		ProcessNode(scene->mRootNode, scene);
 	}

	const std::vector<AssimpVertex>& AssimpLoader::GetVertices() const
	{
		return m_vertices;
	}

	const std::vector<uint32_t>& AssimpLoader::GetIndices() const
	{
		return m_indices;
	}

	const std::vector<AssimpMeshSubset>& AssimpLoader::GetSubsets() const
	{
		return m_subsets;
	}

	const std::vector<AssimpMaterialPaths>& AssimpLoader::GetMaterials() const
	{
		return m_materials;
	}

	const std::vector<AssimpMaterialPathsPBR>& AssimpLoader::GetMaterialsPBR() const
	{
		return m_materialsPBR;
	}

	void AssimpLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			AssimpVertex vert{};
			vert.position = mesh->mVertices[i];
			//vert.position[0] = mesh->mVertices[i].x;
			//vert.position[1] = mesh->mVertices[i].y;
			//vert.position[2] = mesh->mVertices[i].z;

			vert.normal = mesh->mNormals[i];
			//vert.normal[0] = mesh->mNormals[i].x;
			//vert.normal[1] = mesh->mNormals[i].y;
			//vert.normal[2] = mesh->mNormals[i].z;

			if (mesh->mTextureCoords[0])
			{
				vert.uv.x = mesh->mTextureCoords[0][i].x;
				vert.uv.y = mesh->mTextureCoords[0][i].y;
			}

			if (mesh->mTangents)
				vert.tangent = mesh->mTangents[i];
			if (mesh->mBitangents)
				vert.bitangent = mesh->mBitangents[i];

			m_vertices.push_back(vert);
		}

		unsigned int indicesThisMesh = 0;
		for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		{
			aiFace face = mesh->mFaces[i];

			for (unsigned int j = 0; j < face.mNumIndices; ++j)
			{
				m_indices.push_back(face.mIndices[j]);
				++indicesThisMesh;
			}

		}

		// Init subset (verts first
		AssimpMeshSubset subsetData{};
		subsetData.vertexStart = m_meshVertexCount;
		m_meshVertexCount += mesh->mNumVertices;

		subsetData.indexCount = indicesThisMesh;
		subsetData.indexStart = m_meshIndexCount;
		m_meshIndexCount += indicesThisMesh;


		const std::string directory = m_filePath.parent_path().string() + "/";

		// Get Phong Material
		if (!m_PBR)
		{
			auto mtl = scene->mMaterials[mesh->mMaterialIndex];
			aiString diffPath, norPath, opacityPath, specularPath;
			mtl->GetTexture(aiTextureType_DIFFUSE, 0, &diffPath);

			aiReturn norRet = mtl->GetTexture(aiTextureType_NORMALS, 0, &norPath);
			if (norRet != aiReturn_SUCCESS)
				mtl->GetTexture(aiTextureType_HEIGHT, 0, &norPath);

			mtl->GetTexture(aiTextureType_OPACITY, 0, &opacityPath);
			mtl->GetTexture(aiTextureType_SPECULAR, 0, &specularPath);

			// Subset data
			AssimpMaterialPaths matPaths;
			matPaths.diffuseFilePath = (diffPath.length == 0) ? std::nullopt : std::optional<std::string>(std::string(directory + diffPath.C_Str()));
			matPaths.normalFilePath = (norPath.length == 0) ? std::nullopt : std::optional<std::string>(std::string(directory + norPath.C_Str()));
			matPaths.opacityFilePath = (opacityPath.length == 0) ? std::nullopt : std::optional<std::string>(std::string(directory + opacityPath.C_Str()));
			matPaths.specularFilePath = (specularPath.length == 0) ? std::nullopt : std::optional<std::string>(std::string(directory + specularPath.C_Str()));

			subsetData.mats = matPaths;
		}
		// Get PBR material
		else
		{
			auto mtl = scene->mMaterials[mesh->mMaterialIndex];
			aiString albedoPath, norPath, metallicPath, roughnessPath, aoPath, emissionPath;

			mtl->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &albedoPath);

			aiReturn norRet = mtl->GetTexture(aiTextureType_NORMALS, 0, &norPath);
			if (norRet != aiReturn_SUCCESS)
				mtl->GetTexture(aiTextureType_HEIGHT, 0, &norPath);

			mtl->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &metallicPath);
			mtl->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &roughnessPath);
			mtl->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &aoPath);
			mtl->GetTexture(aiTextureType_EMISSIVE, 0, &emissionPath);

			aiString paths[aiTextureType_UNKNOWN];
			for (int x = 0; x < aiTextureType_UNKNOWN; ++x)
			{
				mtl->GetTexture((aiTextureType)(x + 1), 0, &paths[x]);
			}


			AssimpMaterialPathsPBR matPaths;
			matPaths.albedo = (albedoPath.length == 0) ? std::nullopt : std::optional<std::string>(std::string(directory + albedoPath.C_Str()));
			matPaths.normal = (norPath.length == 0) ? std::nullopt : std::optional<std::string>(std::string(directory + norPath.C_Str()));
			matPaths.metallicAndRoughness = (metallicPath.length == 0) ? std::nullopt : std::optional<std::string>(std::string(directory + metallicPath.C_Str()));
			matPaths.ao = (aoPath.length == 0) ? std::nullopt : std::optional<std::string>(std::string(directory + aoPath.C_Str()));
			matPaths.emission = (emissionPath.length == 0) ? std::nullopt : std::optional<std::string>(std::string(directory + emissionPath.C_Str()));

			mtl->Get("$mat.gltf.pbrMetallicRoughness.baseColorFactor", 0, 0, matPaths.baseColorFactor.x);
			mtl->Get("$mat.gltf.pbrMetallicRoughness.baseColorFactor", 0, 1, matPaths.baseColorFactor.y);
			mtl->Get("$mat.gltf.pbrMetallicRoughness.baseColorFactor", 0, 2, matPaths.baseColorFactor.z);

			mtl->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, matPaths.metallicAndRoughnessFactor.x);
			mtl->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, matPaths.metallicAndRoughnessFactor.y);

			subsetData.mats = matPaths;
		}



		m_subsets.push_back(subsetData);
	}

	void AssimpLoader::ProcessNode(aiNode* node, const aiScene* scene)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; ++i)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			ProcessMesh(mesh, scene);
		}

		for (unsigned int i = 0; i < node->mNumChildren; ++i)
			ProcessNode(node->mChildren[i], scene);
	}

}
