#include "Hydra/Render/Material.h"
#include "Hydra/Render/Technique.h"

namespace Hydra
{
	Material::Material(const String & name, SharedPtr<Technique> technique) : _Name(name), _Technique(technique), _CurrentShaderHash(0)
	{
	}

	Material::~Material()
	{
	}

	void Material::SetInt(const String& name, const int& i)
	{

	}

	void Material::SetFloat(const String& name, const float& f)
	{

	}

	void Material::SetBool(const String& name, const bool& b)
	{

	}

	void Material::SetVector2(const String& name, const Vector2& vec)
	{

	}

	void Material::SetVector3(const String& name, const Vector3& vec)
	{

	}

	void Material::SetVector4(const String& name, const Vector4& vec)
	{

	}

	void Material::SetMatrix3(const String& name, const Matrix3& mat)
	{

	}

	void Material::SetMatrix4(const String& name, const Matrix4& mat)
	{

	}

	void Material::SetTexture(const String& name, NVRHI::TextureHandle texture)
	{

	}

	void Material::SetSampler(const String& name, NVRHI::SamplerHandle sampler)
	{

	}

	void Material::SetKeyword(const String& name, bool value)
	{
		auto it = Find(_EnabledKeywords, name);

		bool existInArr = it != _EnabledKeywords.end();

		if (existInArr && value == false)
		{
			_EnabledKeywords.erase(it);

			UpdateHashAndData();
		}

		if (!existInArr && value)
		{
			_EnabledKeywords.emplace_back(name);

			UpdateHashAndData();
		}
	}

	void Material::UpdateHashAndData()
	{
		_CurrentShaderHash = _Technique->GetKeywordHash(_EnabledKeywords);
	}
}