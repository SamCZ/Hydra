#include "Hydra/Render/Material.h"

#include "Hydra/Render/Pipeline/BindingHelpers.h"
#include "Hydra/Render/Technique.h"
#include "Hydra/Render/Shader.h"
#include "Hydra/Engine.h"

namespace Hydra
{
	Map<String, SharedPtr<Technique>> Material::_TechniqueCache;
	Map<String, MaterialPtr> Material::AllMaterials;

	Material::Material(const String & name, SharedPtr<Technique> technique) : Name(name), _Technique(technique), IsInternalMaterial(false)
	{
		if (_Technique->IsPrecompiled())
		{
			Map<String, String> emptyDefs;

			List<Shader*> newShaders = _Technique->GetShaders(emptyDefs, false);

			SetActiveShaderVars(newShaders, 0);

			for (Shader* shader : newShaders)
			{
				_ActiveShaders[shader->GetType()] = shader;
			}
		}
	}

	Material::~Material()
	{
		ITER(_Variables, it)
		{
			delete[] it->second->Data;
		}

		_Variables.clear();

		ITER(_ShaderVarsForVaryingShaders, it0)
		{
			ITER(it0->second, it1)
			{
				ShaderVars* vars = it1->second;

				for (int i = 0; i < vars->ConstantBufferCount; i++)
				{
					RawShaderConstantBuffer& cbuffer = vars->ConstantBuffers[i];

					delete[] cbuffer.LocalDataBuffer;

					Engine::GetRenderInterface()->destroyConstantBuffer(cbuffer.ConstantBuffer);
				}

				delete[] vars->ConstantBuffers;
				delete vars;
			}
		}
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

	void Material::SetStruct(const String & name, StorageStruct & s, size_t size)
	{
		SetVariable(name, VarType::StorageStruct, (void*)(&s), size);
	}

	void Hydra::Material::SetStructArray(const String & name, void* s, size_t size)
	{
		SetVariable(name, VarType::StorageStructArray, s, size);
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

	void Material::SetBuffer(const String & name, NVRHI::BufferHandle buffer)
	{
		if (_BufferVariables.find(name) == _BufferVariables.end())
		{
			_BufferVariables[name] = {};
		}

		BufferVar& var = _BufferVariables[name];
		var.Handle = buffer;
		var.HasChnaged = true;
	}

	NVRHI::BufferHandle Material::GetBuffer(const String & name)
	{
		if (_BufferVariables.find(name) != _BufferVariables.end())
		{
			return _BufferVariables[name].Handle;
		}

		return nullptr;
	}

	Var* Hydra::Material::GetRawVar(const String & name)
	{
		if (_Variables.find(name) != _Variables.end())
		{
			return _Variables[name];
		}
		return nullptr;
	}

	unsigned char* Hydra::Material::GetRawVarData(const String & name)
	{
		if (_Variables.find(name) != _Variables.end())
		{
			Var* var = _Variables[name];

			return var->Data;
		}
		return nullptr;
	}

	Map<String, VarType::Type> Hydra::Material::GetVarTypes()
	{
		Map<String, VarType::Type> map;

		ITER(_ActiveShaderVars, it)
		{
			map.insert(it->second->VariableTypes.begin(), it->second->VariableTypes.end());
		}

		return map;
	}

	void Material::SetDefine(const String& name, const String& value)
	{
		bool updateShader = false;

		auto it = _Defines.find(name);

		if (it != _Defines.end())
		{
			if (_Defines[name] != value)
			{
				updateShader = true;
			}
			_Defines[name] = value;
		}
		else
		{
			updateShader = true;
			_Defines[name] = value;
		}

		if (updateShader)
		{
			_ActiveShaders.clear();

			uint32 shadersId = _Technique->GetDefinesHash(_Defines);

			List<Shader*> newShaders = _Technique->GetShaders(_Defines, false);

			SetActiveShaderVars(newShaders, shadersId);

			for (Shader* shader : newShaders)
			{
				_ActiveShaders[shader->GetType()] = shader;
			}

			ITER(_Variables, it)
			{
				it->second->HasChnaged = true;
			}

			ITER(_TextureVariables, it)
			{
				it->second.HasChnaged = true;
			}

			ITER(_SamplerVariables, it)
			{
				it->second.HasChnaged = true;
			}
		}
	}

	Shader* Material::GetShader(const NVRHI::ShaderType::Enum & type)
	{
		auto it = _ActiveShaders.find(type);

		if (it != _ActiveShaders.end())
		{
			return it->second;
		}

		return nullptr;
	}

	NVRHI::ShaderHandle Material::GetRawShader(const NVRHI::ShaderType::Enum & type)
	{
		Shader* shader = GetShader(type);

		if (shader != nullptr)
		{
			return shader->GetRaw();
		}

		return nullptr;
	}

	void Hydra::Material::ApplyParams(NVRHI::DispatchState & state)
	{
		//TODO: Optimize this method

		// Prepare constant buffers
		_VarsToMarkClean.clear();

		for (Map<NVRHI::ShaderType::Enum, ShaderVars*>::iterator it0 = _ActiveShaderVars.begin(); it0 != _ActiveShaderVars.end(); it0++)
		{
			ShaderVars* vars = it0->second;

			// Write variable data to constant buffer
			for (FastMap<String, RawShaderVariable>::iterator it1 = vars->Variables.begin(); it1 != vars->Variables.end(); it1++)
			{
				String name = it1->first;
				RawShaderVariable& var = it1->second;

				if (_Variables.find(name) != _Variables.end())
				{
					Var* localVar = _Variables[name];

					if (localVar->HasChnaged)
					{
						_VarsToMarkClean.push_back(localVar);

						if (var.Size != localVar->DataSize)
						{
							LogError("Variable(" + ToString(localVar->DataSize) + ") size is not coresponding with source size(" + ToString(var.Size) + ") !");
							continue;
						}

						memcpy(vars->ConstantBuffers[var.ConstantBufferIndex].LocalDataBuffer + var.ByteOffset, localVar->Data, localVar->DataSize);

						vars->ConstantBuffers[var.ConstantBufferIndex].MarkUpdate = true;
					}
				}
			}

			for (int i = 0; i < vars->ConstantBufferCount; i++)
			{
				if (vars->ConstantBuffers[i].MarkUpdate)
				{
					Engine::GetRenderInterface()->writeConstantBuffer(vars->ConstantBuffers[i].ConstantBuffer, vars->ConstantBuffers[i].LocalDataBuffer, vars->ConstantBuffers[i].Size);
					vars->ConstantBuffers[i].MarkUpdate = false;
				}
			}

			NVRHI::PipelineStageBindings* bindigs = &state;

			if (bindigs == nullptr) continue;

			for (int i = 0; i < vars->ConstantBufferCount; i++)
			{
				NVRHI::BindConstantBuffer(*bindigs, vars->ConstantBuffers[i].BindIndex, vars->ConstantBuffers[i].ConstantBuffer);
			}


			for (FastMap<String, RawShaderTextureDefine>::iterator it = vars->TextureDefines.begin(); it != vars->TextureDefines.end(); it++)
			{
				RawShaderTextureDefine& texDefine = it->second;

				if (_TextureVariables.find(it->first) != _TextureVariables.end())
				{
					texDefine.TextureHandle = _TextureVariables[it->first].Handle;
				}

				bool writable = false;

				if (texDefine.TextureHandle)
				{
					writable = texDefine.TextureHandle->GetDesc().isUAV && texDefine.IsWritable;
				}

				NVRHI::BindTexture(*bindigs, texDefine.BindIndex, texDefine.TextureHandle, writable);
			}

			for (FastMap<String, RawShaderSamplerDefine>::iterator it = vars->SamplerDefines.begin(); it != vars->SamplerDefines.end(); it++)
			{
				RawShaderSamplerDefine& samDefine = it->second;

				if (_SamplerVariables.find(it->first) != _SamplerVariables.end())
				{
					samDefine.SamplerHandle = _SamplerVariables[it->first].Handle;
				}

				NVRHI::BindSampler(*bindigs, samDefine.BindIndex, it->second.SamplerHandle);
			}

			for (auto& it : vars->BufferDefines)
			{
				RawShaderBuffer& buffDefine = it.second;

				if (_BufferVariables.find(it.first) != _BufferVariables.end())
				{
					buffDefine.Buffer = _BufferVariables[it.first].Handle;
				}

				bool writable = false;

				if (buffDefine.Buffer)
				{
					writable = buffDefine.Buffer->GetDesc().canHaveUAVs && buffDefine.IsWritable;
				}

				NVRHI::BindBuffer(*bindigs, buffDefine.BindIndex, buffDefine.Buffer, writable);
			}
		}

		for (Var* var : _VarsToMarkClean)
		{
			var->HasChnaged = false;
		}
	}

	void Material::ApplyParams(NVRHI::DrawCallState& state)
	{
		//TODO: Optimize this method

		// Prepare constant buffers
		_VarsToMarkClean.clear();

		for(Map<NVRHI::ShaderType::Enum, ShaderVars*>::iterator it0 = _ActiveShaderVars.begin(); it0 != _ActiveShaderVars.end(); it0++)
		{
			ShaderVars* vars = it0->second;

			// Write variable data to constant buffer
			for(FastMap<String, RawShaderVariable>::iterator it1 = vars->Variables.begin(); it1 != vars->Variables.end(); it1++)
			{
				String name = it1->first;
				RawShaderVariable& var = it1->second;

				if (_Variables.find(name) != _Variables.end())
				{
					Var* localVar = _Variables[name];

					if (localVar->HasChnaged)
					{
						_VarsToMarkClean.push_back(localVar);

						if (var.Size != localVar->DataSize)
						{
							LogError("Variable(" + ToString(localVar->DataSize) + ") size is not coresponding with source size(" + ToString(var.Size) + ") !");
							continue;
						}

						memcpy(vars->ConstantBuffers[var.ConstantBufferIndex].LocalDataBuffer + var.ByteOffset, localVar->Data, localVar->DataSize);

						vars->ConstantBuffers[var.ConstantBufferIndex].MarkUpdate = true;
					}
				}
			}

			for (int i = 0; i < vars->ConstantBufferCount; i++)
			{
				if (vars->ConstantBuffers[i].MarkUpdate)
				{
					Engine::GetRenderInterface()->writeConstantBuffer(vars->ConstantBuffers[i].ConstantBuffer, vars->ConstantBuffers[i].LocalDataBuffer, vars->ConstantBuffers[i].Size);
					vars->ConstantBuffers[i].MarkUpdate = false;
				}
			}

			NVRHI::PipelineStageBindings* bindigs = GetPipelineStageBindingsForShaderType(state, vars->ShaderType);

			if (bindigs == nullptr) continue;

			for (int i = 0; i < vars->ConstantBufferCount; i++)
			{
				NVRHI::BindConstantBuffer(*bindigs, vars->ConstantBuffers[i].BindIndex, vars->ConstantBuffers[i].ConstantBuffer);
			}


			for(FastMap<String, RawShaderTextureDefine>::iterator it = vars->TextureDefines.begin(); it != vars->TextureDefines.end(); it++)
			{
				RawShaderTextureDefine& texDefine = it->second;

				if (_TextureVariables.find(it->first) != _TextureVariables.end())
				{
					texDefine.TextureHandle = _TextureVariables[it->first].Handle;
				}

				NVRHI::BindTexture(*bindigs, texDefine.BindIndex, texDefine.TextureHandle);
			}

			for (FastMap<String, RawShaderSamplerDefine>::iterator it = vars->SamplerDefines.begin(); it != vars->SamplerDefines.end(); it++)
			{
				RawShaderSamplerDefine& samDefine = it->second;

				if (_SamplerVariables.find(it->first) != _SamplerVariables.end())
				{
					samDefine.SamplerHandle = _SamplerVariables[it->first].Handle;
				}

				NVRHI::BindSampler(*bindigs, it->second.BindIndex, it->second.SamplerHandle);
			}

			for (auto& it : vars->BufferDefines)
			{
				RawShaderBuffer& buffDefine = it.second;

				if (_BufferVariables.find(it.first) != _BufferVariables.end())
				{
					buffDefine.Buffer = _BufferVariables[it.first].Handle;
				}

				NVRHI::BindBuffer(*bindigs, buffDefine.BindIndex, buffDefine.Buffer, false);
			}
		}

		for (Var* var : _VarsToMarkClean)
		{
			var->HasChnaged = false;
		}


	}

	SharedPtr<Material> Material::CreateOrGet(const String & name, const File & source, bool precompile, bool isInternalMaterial)
	{
		if (AllMaterials.find(name) != AllMaterials.end())
		{
			return AllMaterials[name];
		}

		SharedPtr<Technique> tech = nullptr;

		if (!source.IsExist())
		{
			throw;
		}

		String fileName = source.GetPath();

		if (_TechniqueCache.find(fileName) != _TechniqueCache.end())
		{
			tech = _TechniqueCache[fileName];
		}
		else
		{
			tech = MakeShared<Technique>(source, precompile);
			_TechniqueCache[fileName] = tech;
		}

		MaterialPtr material = MakeShared<Material>(name, tech);

		material->IsInternalMaterial = isInternalMaterial;

		AllMaterials[name] = material;

		return material;
	}

	SharedPtr<Material> Material::CreateOrGet(const File & source, bool precompile, bool isInternalMaterial)
	{
		return CreateOrGet(source.GetPath(), source, precompile, isInternalMaterial);
	}

	NVRHI::PipelineStageBindings* Material::GetPipelineStageBindingsForShaderType(NVRHI::DrawCallState& state, const NVRHI::ShaderType::Enum & type)
	{
		switch (type)
		{
		case NVRHI::ShaderType::SHADER_VERTEX:
			return &state.VS;
			break;
		case NVRHI::ShaderType::SHADER_HULL:
			return &state.HS;
			break;
		case NVRHI::ShaderType::SHADER_DOMAIN:
			return &state.DS;
			break;
		case NVRHI::ShaderType::SHADER_GEOMETRY:
			return &state.GS;
			break;
		case NVRHI::ShaderType::SHADER_PIXEL:
			return &state.PS;
			break;
		default:
			LogError("Material::GetPipelineStageBindingsForShaderType", "Cannot find pipeline stage bindings for shader type : " + ToString((int)type));
			return nullptr;
		}
	}

	void Material::SetActiveShaderVars(List<Shader*>& shaders, uint32 packId)
	{
		if (_ShaderVarsForVaryingShaders.find(packId) != _ShaderVarsForVaryingShaders.end())
		{
			_ActiveShaderVars = _ShaderVarsForVaryingShaders[packId];
		}
		else
		{
			_ActiveShaderVars.clear();

			for (Shader* shader : shaders)
			{
				ShaderVars* vars = shader->CreateShaderVars();

				for (int i = 0; i < vars->ConstantBufferCount; i++)
				{
					RawShaderConstantBuffer& cbuffer = vars->ConstantBuffers[i];

					cbuffer.LocalDataBuffer = new unsigned char[cbuffer.Size];
					ZeroMemory(cbuffer.LocalDataBuffer, cbuffer.Size);

					cbuffer.ConstantBuffer = Engine::GetRenderInterface()->createConstantBuffer(NVRHI::ConstantBufferDesc(cbuffer.Size, cbuffer.Name.c_str()), nullptr);
				}

				_ActiveShaderVars[shader->GetType()] = vars;
			}

			_ShaderVarsForVaryingShaders[packId] = _ActiveShaderVars;
		}
	}

	bool Material::SetVariable(const String & name, const VarType::Type & type, const void* data, size_t size)
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

}