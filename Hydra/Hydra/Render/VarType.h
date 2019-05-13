#pragma once

struct VarType
{
	enum Type
	{
		Int,
		UInt,
		Float,
		Bool,
		Vector2,
		Vector3,
		Vector4,
		Matrix3,
		Matrix4,

		Vector4Array,
		StorageStruct,
		StorageStructArray
	};
};