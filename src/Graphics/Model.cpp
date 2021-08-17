#include "pch.h"
#include "Graphics/Model.h"

namespace Gino
{
    Model::Model() :
        Component(ComponentType::ModelType)
    {
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

