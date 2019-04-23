#pragma once

#include <d3d11.h>

#include "Hydra/Core/Common.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"
#include "Hydra/Render/VarType.h"

struct RawShaderVariable
{
	unsigned int ByteOffset;
	unsigned int Size;
	unsigned int ConstantBufferIndex;
};

struct RawShaderConstantBuffer
{
	String Name;
	unsigned int Size;
	unsigned int BindIndex;
	NVRHI::ConstantBufferHandle ConstantBuffer;
	unsigned char* LocalDataBuffer;
	List<RawShaderVariable> Variables;
	bool MarkUpdate;
};

struct RawShaderTextureDefine
{
	unsigned int Index;
	unsigned int BindIndex;
	NVRHI::TextureHandle TextureHandle;
	bool IsWritable;
};

struct RawShaderBuffer
{
	String Name;
	unsigned int Size;
	unsigned int Index;
	unsigned int BindIndex;
	bool IsWritable;
	NVRHI::BufferHandle Buffer;
};

struct RawShaderSamplerDefine
{
	unsigned int Index;
	unsigned int BindIndex;
	NVRHI::SamplerHandle SamplerHandle;
};

struct ShaderVars
{
	NVRHI::ShaderType::Enum ShaderType;

	RawShaderConstantBuffer* ConstantBuffers;
	int ConstantBufferCount;

	FastMap<String, RawShaderTextureDefine> TextureDefines;
	FastMap<String, RawShaderSamplerDefine> SamplerDefines;
	FastMap<String, RawShaderVariable> Variables;
	FastMap<String, RawShaderBuffer> BufferDefines;

	Map<String, VarType::Type> VariableTypes;
};

class HYDRA_API Shader
{
private:
	String _Name;
	NVRHI::ShaderType::Enum _Type;
	NVRHI::ShaderHandle _Handle;
	ID3DBlob* _Blob;

	ShaderVars* _LocalShaderVarCache;

public:
	Shader(const String& name, const NVRHI::ShaderType::Enum& type, NVRHI::ShaderHandle shaderHandle, ID3DBlob* shaderBlob);
	~Shader();

	NVRHI::ShaderHandle GetRaw();
	ID3DBlob* GetBlob();

	NVRHI::ShaderType::Enum GetType();

	ShaderVars* CreateShaderVars();

	NVRHI::InputLayoutHandle CreateInputLayout();

private:
	void Initialize();
};