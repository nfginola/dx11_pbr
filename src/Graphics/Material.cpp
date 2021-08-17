#include "pch.h"
#include "Graphics/ResourceTypes.h"
#include "Graphics/Material.h"

namespace Gino
{
    void Material::Initialize(const PhongMaterialData& data)
    {
        m_type = MaterialType::Phong;
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
}
