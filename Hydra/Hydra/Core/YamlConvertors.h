#pragma once

#include "Hydra/Core/Vector.h"
#include "yaml-cpp/yaml.h"

namespace YAML
{
	template<>
	struct convert<Vector2>
	{
		static Node encode(const Vector2& rhs)
		{
			Node node;
			node.SetStyle(EmitterStyle::Flow);
			node["x"] = rhs.x;
			node["y"] = rhs.y;
			return node;
		}

		static bool decode(const Node& node, Vector2& rhs)
		{
			if (!node.IsMap() || node.size() != 2)
			{
				return false;
			}

			rhs.x = node["x"].as<float>();
			rhs.y = node["y"].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Vector3>
	{
		static Node encode(const Vector3& rhs)
		{
			Node node;
			node.SetStyle(EmitterStyle::Flow);
			node["x"] = rhs.x;
			node["y"] = rhs.y;
			node["z"] = rhs.z;
			return node;
		}

		static bool decode(const Node& node, Vector3& rhs)
		{
			if (!node.IsMap() || node.size() != 3)
			{
				return false;
			}

			rhs.x = node["x"].as<float>();
			rhs.y = node["y"].as<float>();
			rhs.z = node["z"].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Vector4>
	{
		static Node encode(const Vector4& rhs)
		{
			Node node;
			node.SetStyle(EmitterStyle::Flow);
			node["x"] = rhs.x;
			node["y"] = rhs.y;
			node["z"] = rhs.z;
			node["w"] = rhs.w;
			return node;
		}

		static bool decode(const Node& node, Vector4& rhs)
		{
			if (!node.IsMap() || node.size() != 4)
			{
				return false;
			}

			rhs.x = node["x"].as<float>();
			rhs.y = node["y"].as<float>();
			rhs.z = node["z"].as<float>();
			rhs.w = node["w"].as<float>();
			return true;
		}
	};
}