#pragma once

#include "Hydra/Core/String.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"

struct ShaderVertexInputDefinition
{
	String SemanticName;
	int SemanticIndex;
	NVRHI::Format::Enum Format;
	bool Instanced;

	inline bool operator==(const ShaderVertexInputDefinition& other)
	{
		return SemanticName == other.SemanticName && SemanticIndex == other.SemanticIndex && Format == other.Format && Instanced == other.Instanced;
	}

	inline String ToHash()
	{
		return SemanticName + String(":") + ToString(SemanticIndex) + String(":") + ToString((int)Format) + String(":") + ToString(Instanced);
	}

	inline String Print()
	{
		return String("ShaderVertexInputDefinition(") + "Name=" + SemanticName + ", SemanticIndex=" + ToString(SemanticIndex) + ", Format=" + ToString((int)Format) + ", IsInstanced=" + ToString(Instanced);
	}
};