#pragma once

namespace Gino
{
	struct Texture;

	enum class MaterialType
	{
		Phong,
		PBR
	};

	struct PhongMaterialData
	{
		Texture* diffuse = nullptr;
		Texture* specular = nullptr;
		Texture* opacity = nullptr;
		Texture* normal = nullptr;

		// Other misc. data (e.g colors) can be stored here too
	};

	struct PBRMaterialData
	{
		Texture* albedo = nullptr;
		Texture* normal = nullptr;
		Texture* metallicAndRoughness = nullptr;
		Texture* ao = nullptr;

		// Other misc. data can be stored here too
	};

	// We use variant to make it simple. No complex material schemes.
	// Idea here is that we can easily modify and extend this should we want other material types 
	// - We add another initialize that takes in another Data type
	// - Add another ENUM
	// - Existing materials undisturbed!
	class Material
	{
	public:
		Material() = default;
		~Material() = default;

		void Initialize(const PhongMaterialData& data);
		void Initialize(const PBRMaterialData& data);

		template <typename T>
		const T& GetProperties() const;

		MaterialType GetType() const;
	private:
		MaterialType m_type = MaterialType::Phong;
		std::variant<PhongMaterialData, PBRMaterialData> m_data;
		// ShaderGroup and other Pipeline resources? Maybe not?

	};

	template<typename T>
	inline const T& Material::GetProperties() const
	{
		try
		{
			return std::get<T>(m_data);
		}
		catch (std::bad_variant_access const& ex)
		{
			std::cout << ex.what() << " || Gino::Material : GetProperties() is called with an invalid type!\n";
			assert(false);

			// Should never reach here: Here to get rid of the warnings.. 
			return std::get<T>(m_data);
		}
	}

}


