#pragma once

#include "Hydra/Core/String.h"
#include "Hydra/Core/Container.h"
#include "Hydra/Core/SmartPointer.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"

namespace Hydra
{
	class Material;

	struct TextureLayout
	{
		String Name;
		String GroupName;
		String ShaderDefName;
		NVRHI::TextureHandle Texture;
		String ComponentDef;
	};

	class TextureLayoutDef
	{
	private:
		Map<String, TextureLayout> LayoutDefs;
		String ActiveGroup;
	public:

		void BeginGroup(const String& name);
		void EndGroup();

		bool DeleteGroup(const String& name, bool destroyTextures = false);

		void Add(const String& name, const String shaderDefname, NVRHI::TextureHandle texture, const String& componentDef);

		void ApplyToMaterial(const String& name, SharedPtr<Material> material, const String& shaderVarName);
	};

	DEFINE_PTR(TextureLayoutDef)
}