#include "Hydra/Render/Material.h"
#include "Hydra/Render/Technique.h"

namespace Hydra
{
	Material::Material(const String & name, SharedPtr<Technique> technique) : _Name(name), _Technique(technique), _CurrentShaderHash(0)
	{
	}

	Material::~Material()
	{
		ITER(_Variables, it)
		{
			delete[] it->second->Data;
		}

		_Variables.clear();
	}

	void Material::SetInt(const String& name, const int& i)
	{
		SetVariable<int>(name, VarType::Int, i);
	}

	bool Material::GetInt(const String& name, int* outInt)
	{
		return GetVariable(name, VarType::Int, outInt);
	}

	void Material::SetUInt(const String& name, const unsigned int & i)
	{
		SetVariable<unsigned int>(name, VarType::UInt, i);
	}

	bool Material::GetUInt(const String& name, unsigned int * outInt)
	{
		return GetVariable(name, VarType::UInt, outInt);
	}

	void Material::SetFloat(const String& name, const float& f)
	{
		SetVariable<float>(name, VarType::Float, f);
	}

	bool Material::GetFloat(const String& name, float* outFloat)
	{
		return GetVariable<float>(name, VarType::Float, outFloat);
	}

	void Material::SetBool(const String& name, const bool& b)
	{
		SetVariable(name, VarType::Bool, b);
	}

	bool Material::GetBool(const String& name, bool* outBool)
	{
		return GetVariable<bool>(name, VarType::Bool, outBool);
	}

	void Material::SetVector2(const String& name, const Vector2& vec)
	{
		SetVariable(name, VarType::Vector2, vec);
	}

	bool Material::GetVector2(const String& name, Vector2* outVec)
	{
		return GetVariable<Vector2>(name, VarType::Vector2, outVec);
	}

	void Material::SetVector3(const String& name, const Vector3& vec)
	{
		SetVariable(name, VarType::Vector3, vec);
	}

	bool Material::GetVector3(const String & name, Vector3* outVec)
	{
		return GetVariable<Vector3>(name, VarType::Vector3, outVec);
	}

	void Material::SetVector4(const String& name, const Vector4& vec)
	{
		SetVariable(name, VarType::Vector4, vec);
	}

	bool Material::GetVector4(const String& name, Vector4* outVec)
	{
		return GetVariable<Vector4>(name, VarType::Vector4, outVec);
	}

	void Material::SetVector4Array(const String & name, Vector4* vecArr, size_t arrSize)
	{
		SetVariable(name, VarType::Vector4Array, vecArr, sizeof(Vector4) * arrSize);
	}

	bool Material::GetVector4Array(const String & name, Vector4* vector, size_t arrSize)
	{
		return GetVariable(name, VarType::Vector4Array, vector, false, sizeof(Vector4) * arrSize);
	}

	void Material::SetMatrix3(const String& name, const Matrix3& mat)
	{
		SetVariable(name, VarType::Matrix3, mat);
	}

	bool Material::GetMatrix3(const String& name, Matrix3* outMat)
	{
		return GetVariable<Matrix3>(name, VarType::Matrix3, outMat);
	}

	void Material::SetMatrix4(const String& name, const Matrix4& mat)
	{
		SetVariable(name, VarType::Matrix4, mat);
	}

	bool Material::GetMatrix4(const String& name, Matrix4* outMat)
	{
		return GetVariable<Matrix4>(name, VarType::Matrix4, outMat);
	}


	void Material::SetTexture(const String& name, NVRHI::TextureHandle texture)
	{
		if (_TextureVariables.find(name) == _TextureVariables.end())
		{
			_TextureVariables[name] = {};
		}

		TextureVar& var = _TextureVariables[name];
		var.Handle = texture;
		var.HasChnaged = true;
	}

	NVRHI::TextureHandle Material::GetTexture(const String& name)
	{
		if (_TextureVariables.find(name) != _TextureVariables.end())
		{
			return _TextureVariables[name].Handle;
		}

		return nullptr;
	}

	void Material::SetSampler(const String& name, NVRHI::SamplerHandle sampler)
	{
		if (_SamplerVariables.find(name) == _SamplerVariables.end())
		{
			_SamplerVariables[name] = {};
		}

		SamplerVar& var = _SamplerVariables[name];
		var.Handle = sampler;
		var.HasChnaged = true;
	}

	NVRHI::SamplerHandle Material::GetSampler(const String& name)
	{
		if (_SamplerVariables.find(name) != _SamplerVariables.end())
		{
			return _SamplerVariables[name].Handle;
		}

		return nullptr;
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

	void Material::SetDefine(const String& name, const String& value)
	{

	}

	SharedPtr<Material> Hydra::Material::CreateOrGet(const String & name, const File & source, bool doNotPreCompile)
	{
		//TODO: Material loading
		SharedPtr<Technique> tech = nullptr;
		return MakeShared<Material>("0", tech);
	}

	SharedPtr<Material> Hydra::Material::CreateOrGet(const File & source, bool doNotPreCompile)
	{
		//TODO: Material loading
		SharedPtr<Technique> tech = nullptr;
		return MakeShared<Material>("0", tech);
	}

	bool Hydra::Material::SetVariable(const String & name, const VarType::Type & type, const void* data, size_t size)
	{
		Var* var = nullptr;

		if (_Variables.find(name) != _Variables.end())
		{
			var = _Variables[name];
		}
		else
		{
			var = new Var();
			var->Name = name;
			var->Type = type;
			var->DataSize = size;
			var->Data = new unsigned char[size];
		}

		if (var->Type != type)
		{
			LogError("Material::SetVariable", name + ", " + ToString((int)type), "Type cannot be chnaged from(" + ToString((int)var->Type) + ") to(" + ToString((int)type) + ") !");
			return false;
		}

		if (var->DataSize != size)
		{
			LogError("Material::SetVariable", name + ", " + ToString((int)type), "Illegal type detected ! Old data size(" + ToString(var->DataSize) + ") new(" + ToString(size) + ")");
			return false;
		}

		memcpy(var->Data, data, size);

		var->HasChnaged = true;

		_Variables[name] = var;

		return true;
	}

	void Material::UpdateHashAndData()
	{
		//_CurrentShaderHash = _Technique->GetKeywordHash(_EnabledKeywords);
	}
}