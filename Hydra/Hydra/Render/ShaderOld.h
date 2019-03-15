#if 0
#pragma once

#include <d3dcommon.h>

#include "Hydra/Core/String.h"
#include "Hydra/Core/Container.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"

namespace Hydra
{
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
	};

	struct RawShaderSamplerDefine
	{
		unsigned int Index;
		unsigned int BindIndex;
		NVRHI::SamplerHandle SamplerHandle;
	};

	class Shader
	{
	private:
		const String& _Name;
		NVRHI::ShaderHandle _ShaderHandle;
		NVRHI::InputLayoutHandle _InputLayoutHandle;
		ID3DBlob* _ShaderBlob;

		RawShaderConstantBuffer* _ConstantBuffers;
		int _ConstantBufferCount;

		FastMap<String, RawShaderTextureDefine> _TextureDefines;
		FastMap<String, RawShaderSamplerDefine> _SamplerDefines;
		FastMap<String, RawShaderVariable> _Variables;
	public:
		Shader(const String& name, NVRHI::ShaderHandle shaderHandle, ID3DBlob* shaderBlob);
		~Shader();

		NVRHI::ShaderHandle GetHandle();
		ID3DBlob* GetBlob();

		NVRHI::InputLayoutHandle GetOrGenerateInputLayout();

		bool SetVariable(const String& name, const void* data, unsigned int size);
		bool IsVariableExists(const String& name);
		bool IsVariableExists(const String& name, unsigned int size);

		template <typename T>
		inline bool SetVariable(const String name, T data)
		{
			return SetVariable(name, (void*)(&data), sizeof(data));
		}

		template <typename T>
		inline T GetVariable(const String name)
		{
			if (!IsVariableExists(name, sizeof(T))) return T();

			RawShaderVariable& varDef = _Variables[name];

			return (T)(_ConstantBuffers[varDef.ConstantBufferIndex].LocalDataBuffer + varDef.ByteOffset);
		}

		void UploadVariableData();
		void BindConstantBuffers(NVRHI::PipelineStageBindings& binding);

		void SetTexture(const String& name, NVRHI::TextureHandle texture);
		void SetSampler(const String& name, NVRHI::SamplerHandle sampler);
		
		bool IsTextureExists(const String& name);
		bool IsSamplerExists(const String& name);

		NVRHI::TextureHandle GetTexture(const String& name);
		NVRHI::SamplerHandle GetSampler(const String& name);

		void BindTextures(NVRHI::PipelineStageBindings& binding);
		void BindSamplers(NVRHI::PipelineStageBindings& binding);

		void BindAllData(NVRHI::PipelineStageBindings& binding);

		String GetName();

	private:
		void Initialize();
	};

	typedef Shader* ShaderPtr;
}
#endif