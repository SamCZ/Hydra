#include "AssetManager.h"

#include "Hydra/Core/Log.h"
#include "Hydra/Core/json.h"

#include "Hydra/Render/Technique.h"

#include "Hydra/Assets/Importers/TextureImporter.h"
#include "Hydra/Core/Stream/FileStream.h"

#include "Hydra/Framework/StaticMesh.h"
#include "Hydra/Assets/Importers/ModelImporter.h"

static String ImageExtensions[]{
	"png", "gif", "jpg", "jpeg", "tiff"
};

Json FromVec2(const glm::vec2& v2)
{
	Json obj = Json::object();
	obj["x"] = v2.x;
	obj["y"] = v2.y;
	return obj;
}
Json FromVec3(const glm::vec3& v3)
{
	Json obj = Json::object();
	obj["x"] = v3.x;
	obj["y"] = v3.y;
	obj["z"] = v3.z;
	return obj;
}
Json FromVec4(const glm::vec4& v4)
{
	Json obj = Json::object();
	obj["x"] = v4.x;
	obj["y"] = v4.y;
	obj["z"] = v4.z;
	obj["w"] = v4.w;
	return obj;
}

Vector2 ToVec2(Json& json)
{
	return Vector2(json["x"].get<float>(), json["y"].get<float>());
}
Vector3 ToVec3(Json& json)
{
	return Vector3(json["x"].get<float>(), json["y"].get<float>(), json["z"].get<float>());
}
Vector4 ToVec4(Json& json)
{
	return Vector4(json["x"].get<float>(), json["y"].get<float>(), json["z"].get<float>(), json["w"].get<float>());
}

AssetManager::AssetManager(EngineContext* context) : _Context(context)
{
}

AssetManager::~AssetManager()
{
}

void AssetManager::AddAssetLocator(IAssetLocator * locator)
{
}

void AssetManager::AddAssetImporter(IAssetImporter * importer)
{
}

MaterialInterface* AssetManager::GetMaterial(const String & path)
{
	auto iter = _Materials.find(path);

	if (iter != _Materials.end())
	{
		return iter->second;
	}

	return nullptr;
}

NVRHI::TextureHandle AssetManager::GetTexture(const String & path)
{
	auto iter = _Textures.find(path);

	if (iter != _Textures.end())
	{
		return iter->second;
	}

	return nullptr;
}

HStaticMesh* AssetManager::GetMesh(const String & path)
{
	FileStream stream = FileStream(path);
	Blob* data = stream.Read();

	List<HAsset*> assets;

	ModelImporter importer;

	ModelImportOptions options;
	options.CombineMeshes = true;
	options.Name = path;

	if (importer.Import(*data, options, assets))
	{
		return assets[0]->SafeCast<HStaticMesh>();
	}

	return nullptr;
}

List<HStaticMesh*> AssetManager::GetMeshParts(const String path)
{
	//TODO: This
	return List<HStaticMesh*>();
}

void AssetManager::LoadProjectFiles()
{
	File projectFolder = File("ProjectFiles");

	for (File file : projectFolder.ListFiles())
	{
		Log(file);

		String ext = file.GetExtension();

		if (ext == "mat")
		{
			LoadMaterial(file);
		}

		for (String imgExt : ImageExtensions)
		{
			if (ext == imgExt)
			{
				LoadTexture(file);
				break;
			}
		}
	}

	File engineFolder = File("Assets");

	for (File file : engineFolder.ListFiles())
	{
		String ext = file.GetExtension();

		if (ext == "mat")
		{
			LoadMaterial(file);
		}

		for (String imgExt : ImageExtensions)
		{
			if (ext == imgExt)
			{
				LoadTexture(file);
				break;
			}
		}
	}
}

Json ReadJson(const File& file)
{
	std::ifstream i(file.GetPath());
	if (!i.is_open())
	{
		return NULL;
	}
	Json json;
	i >> json;
	i.close();
	return json;
}

SharedPtr<Technique> AssetManager::LoadTechnique(const File& file)
{
	auto iter = _Techniques.find(file);

	if (iter != _Techniques.end())
	{
		return iter->second;
	}

	SharedPtr<Technique> technique = MakeShared<Technique>(_Context, file, true);

	_Techniques[file] = technique;

	return technique;
}

void AssetManager::LoadMaterial(const File& file)
{
	auto iter = _Materials.find(file);

	if (iter != _Materials.end())
	{
		return;
	}

	Json json = ReadJson(file);

	if (json.find("Shader") != json.end())
	{
		MaterialInterface* mat = nullptr;

		String shaderPath = json["Shader"].get<std::string>();

		SharedPtr<Technique> technique = LoadTechnique(shaderPath);

		if (technique == nullptr) return;

		mat = new MaterialInterface(file.GetPath(), technique);

		for (auto it = json["Defines"].begin(); it != json["Defines"].end(); it++)
		{
			std::string defineName = it.key();
			bool defineValue = it->get<bool>();

			//TODO: Defines
		}

		for (auto it = json["Params"].begin(); it != json["Params"].end(); it++)
		{
			std::string name = it.key();
			int dIndex = name.find_first_of(':');
			std::string type = name.substr(0, dIndex);
			name = name.substr(dIndex + 1);

			if (type == "Bool")
			{
				mat->SetBool(name, it->get<bool>());
			}
			else if (type == "Float")
			{
				mat->SetFloat(name, it->get<float>());
			}
			else if (type == "Int")
			{
				mat->SetInt(name, it->get<int>());
			}
			else if (type == "Texture")
			{
				String texturePath = it->get<std::string>();

				//TODO: Material textures
			}
			else if (type == "Vector2")
			{
				mat->SetVector2(name, ToVec2(*it));
			}
			else if (type == "Vector3")
			{
				mat->SetVector3(name, ToVec3(*it));
			}
			else if (type == "Vector4")
			{
				mat->SetVector4(name, ToVec4(*it));
			}
		}

		_Materials[file] = mat;

		Log("AssetManager::LoadMaterial", file.GetPath(), "Loaded.");
	}
}

void AssetManager::SaveMaterial(MaterialInterface* material)
{
	//TODO: Saving material

	/*Json json;
	json["Shader"] = material->getName();

	Json params = Json::object();
	for (const auto& i : material->getTechnique()->getParams())
	{
		std::string name = i.first;
		MaterialParam* param = i.second;
		if (param->Type == VarType::Bool)
		{
			params["Bool:" + param->Name] = param->getData<bool>();
		}
		else if (param->Type == VarType::Float)
		{
			params["Float:" + param->Name] = param->getData<float>();
		}
		else if (param->Type == VarType::Int)
		{
			params["Int:" + param->Name] = param->getData<int>();
		}
		else if (param->Type == VarType::Texture)
		{
			if (param->getData<MaterialParamTexture>().Texture == nullptr) continue;
			params["Texture:" + param->Name] = param->getData<MaterialParamTexture>().Texture->getFile();
		}
		else if (param->Type == VarType::Vector2)
		{
			params["Vec2:" + param->Name] = fromVec2(param->getData<glm::vec2>());
		}
		else if (param->Type == VarType::Vector3)
		{
			params["Vec3:" + param->Name] = fromVec3(param->getData<glm::vec3>());
		}
		else if (param->Type == VarType::Vector4)
		{
			params["Vec4:" + param->Name] = fromVec4(param->getData<glm::vec4>());
		}
	}
	json["Params"] = params;

	jjson defines = jjson::object();
	for (const auto& i : material->getTechnique()->getDefineSettings())
	{
		defines[i.first] = i.second;
	}
	json["Defines"] = defines;

	jjson jSTate = jjson::object();

	RenderState& state = material->getRenderState();
	jSTate["FaceCull"] = (int)state.faceCull;
	jSTate["DepthTest"] = state.depthTest;

	json["State"] = jSTate;
	Files::saveJson(json, file);*/
}

void AssetManager::LoadTexture(const File& file)
{
	if (true) return;

	Log("AssetManager::LoadTexture", file, "Trying to load...");

	FileStream stream = FileStream(file);
	Blob* data = stream.Read();

	List<HAsset*> assets;

	TextureImporter importer(_Context);
	
	if (importer.Import(*data, {}, assets))
	{
		NVRHI::TextureHandle tex = assets[0]->SafeCast<NVRHI::ITexture>();

		if (tex)
		{
			_Textures[file] = tex;

			Log("AssetManager::LoadTexture", file, "Loaded.");
		}
	}

	delete data;
}
