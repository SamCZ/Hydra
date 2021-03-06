#pragma once

#include "Hydra/Core/Common.h"
#include "Hydra/Core/Vector.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"
#include "Hydra/Render/Technique.h"

#include <d3d11.h>

#include "Hydra/Render/VarType.h"

#include "Hydra/Assets/Asset.h"

class Technique;
class Shader;
struct ShaderVars;

struct StorageStruct
{

};

struct Var
{
	String Name;
	VarType::Type Type;
	unsigned char* Data;
	size_t DataSize;
	bool HasChnaged;
};

struct TextureVar
{
	NVRHI::TextureHandle Handle;
	bool HasChnaged;
};

struct SamplerVar
{
	NVRHI::SamplerHandle Handle;
	bool HasChnaged;
};

struct BufferVar
{
	NVRHI::BufferHandle Handle;
	bool HasChnaged;
};

class HYDRA_API MaterialInterface : public HAsset
{
	HCLASS_BODY_NO_FNC_POINTER(MaterialInterface)
private:
	static Map<String, SharedPtr<Technique>> _TechniqueCache;

	SharedPtr<Technique> _Technique;

	Map<String, Var*> _Variables;
	Map<String, TextureVar> _TextureVariables;
	Map<String, SamplerVar> _SamplerVariables;
	Map<String, BufferVar> _BufferVariables;

	Map<NVRHI::ShaderType::Enum, Shader*> _ActiveShaders;
	Map<String, String> _Defines;

	Map<uint32, Map<NVRHI::ShaderType::Enum, ShaderVars*>> _ShaderVarsForVaryingShaders;
	Map<NVRHI::ShaderType::Enum, ShaderVars*> _ActiveShaderVars;

	List<Var*> _VarsToMarkClean;
public:
	static Map<String, MaterialInterface*> AllMaterialInterfaces;

	String Name;
	bool IsInternalMaterialInterface;

	static const MaterialInterface NullMaterial;
public:
	MaterialInterface(const String& name, SharedPtr<Technique> technique);
	~MaterialInterface();

	void SetInt(const String& name, const int& i);
	bool GetInt(const String& name, int* outInt);

	void SetUInt(const String& name, const unsigned int& i);
	bool GetUInt(const String& name, unsigned int* outInt);

	void SetFloat(const String& name, const float& f);
	bool GetFloat(const String& name, float* outFloat);

	void SetBool(const String& name, const bool& b);
	bool GetBool(const String& name, bool* outBool);

	void SetVector2(const String& name, const Vector2& vec);
	bool GetVector2(const String& name, Vector2* outVec);

	void SetVector3(const String& name, const Vector3& vec);
	bool GetVector3(const String& name, Vector3* outVec);

	void SetVector4(const String& name, const Vector4& vec);
	bool GetVector4(const String& name, Vector4* outVec);

	void SetVector4Array(const String& name, Vector4* vecArr, size_t arrSize);
	bool GetVector4Array(const String& name, Vector4* vector, size_t arrSize);

	void SetMatrix3(const String& name, const Matrix3& mat);
	bool GetMatrix3(const String& name, Matrix3* outMat);

	void SetMatrix4(const String& name, const Matrix4& mat);
	bool GetMatrix4(const String& name, Matrix4* outMat);

	void SetStruct(const String& name, StorageStruct& s, size_t size);
	void SetStructArray(const String& name, void* s, size_t size);

	void SetTexture(const String& name, NVRHI::TextureHandle texture);
	NVRHI::TextureHandle GetTexture(const String& name);

	void SetSampler(const String& name, NVRHI::SamplerHandle sampler);
	NVRHI::SamplerHandle GetSampler(const String& name);

	void SetBuffer(const String& name, NVRHI::BufferHandle buffer);
	NVRHI::BufferHandle GetBuffer(const String& name);

	Var* GetRawVar(const String& name);
	unsigned char* GetRawVarData(const String& name);

	Map<String, VarType::Type> GetVarTypes();

	void SetDefine(const String& name, const String& value);

	Shader* GetShader(const NVRHI::ShaderType::Enum& type);
	NVRHI::ShaderHandle GetRawShader(const NVRHI::ShaderType::Enum& type);

	void ApplyParams(NVRHI::DispatchState& state);
	void ApplyParams(NVRHI::DrawCallState& state);

	SharedPtr<Technique> GetTechnique();

private:

	NVRHI::PipelineStageBindings* GetPipelineStageBindingsForShaderType(NVRHI::DrawCallState& state, const NVRHI::ShaderType::Enum& type);

	void SetActiveShaderVars(List<Shader*>& shaders, uint32 packId);

	bool SetVariable(const String& name, const VarType::Type& type, const void* data, size_t size);

	template <typename T>
	inline bool GetVariable(const String name, const VarType::Type& type, T* outData, bool autoSize = true, size_t customSize = 0)
	{
		size_t size = sizeof(T);

		if (!autoSize)
		{
			size = customSize;
		}

		if (_Variables.find(name) != _Variables.end())
		{
			Var* var = _Variables[name];

			if (var->Type != type)
			{
				LogError("Material::GetVariable", name + ", " + ToString((int)type), "Type cannot be chnaged from(" + ToString((int)var->Type) + ") to(" + ToString((int)type) + ") !");
				return false;
			}

			if (var->DataSize != size)
			{
				LogError("Material::GetVariable", name + ", " + ToString((int)type), "Illegal type detected ! Old data size(" + ToString(var->DataSize) + ") new(" + ToString(size) + ")");
				return false;
			}

			memcpy(outData, var->Data, var->DataSize);

			return true;
		}

		return false;
	}

	template <typename T>
	inline bool SetVariable(const String name, const VarType::Type& type, T data)
	{
		void* rawData = (void*)(&data);
		size_t size = sizeof(data);

		return SetVariable(name, type, rawData, size);
	}
};