#include "Hydra/Render/TextureLayoutDef.h"
#include "Hydra/Render/Material.h"

namespace Hydra
{
	void TextureLayoutDef::BeginGroup(const String& name)
	{
		if (ActiveGroup != String_None)
		{
			LogError("TextureLayoutDef::BeginGroup", name, "Other group is already set ! (" + ActiveGroup + ")");
			return;
		}

		ActiveGroup = name;
	}

	void TextureLayoutDef::EndGroup()
	{
		ActiveGroup = String_None;
	}
	bool Hydra::TextureLayoutDef::DeleteGroup(const String& name)
	{
		bool deletedSomething = false;

		List<String> toRemove;

		ITER(LayoutDefs, it)
		{
			if (it->second.GroupName == name)
			{
				toRemove.push_back(it->second.Name);
			}
		}

		for (String name : toRemove)
		{
			LayoutDefs.erase(name);
		}

		return deletedSomething;
	}

	void TextureLayoutDef::Add(const String& name, const String shaderDefname, NVRHI::TextureHandle texture, const String & componentDef)
	{
		TextureLayout* layout = nullptr;
		TextureLayout emptyNewLayout = {};

		if (LayoutDefs.find(name) != LayoutDefs.end())
		{
			layout = &LayoutDefs[name];
		}
		else
		{
			layout = &emptyNewLayout;
		}

		layout->GroupName = ActiveGroup;
		layout->ShaderDefName = shaderDefname;
		layout->Texture = texture;
		layout->ComponentDef = componentDef;

		LayoutDefs[name] = *layout;
	}

	void TextureLayoutDef::ApplyToMaterial(const String& name, MaterialInterface* material, const String& shaderVarName)
	{
		if (LayoutDefs.find(name) != LayoutDefs.end())
		{
			TextureLayout& layout = LayoutDefs[name];

			material->SetDefine(layout.ShaderDefName, layout.ComponentDef);
			material->SetTexture(shaderVarName, layout.Texture);
		}
	}

	
}