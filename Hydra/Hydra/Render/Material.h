#pragma once

#include "Hydra/Core/Common.h"
#include "Hydra/Core/Vector.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"

namespace Hydra
{
	class Technique;
	class Shader;

	class Material
	{
	private:
		List<String> _EnabledKeywords;
	public:
		Material();
		~Material();

		void SetInt(const String& name, const int& i);
		void SetFloat(const String& name, const float& f);
		void SetBool(const String& name, const bool& b);

		void SetVector2(const String& name, const Vector2& vec);
		void SetVector3(const String& name, const Vector3& vec);
		void SetVector4(const String& name, const Vector4& vec);

		void SetMatrix3(const String& name, const Matrix3& mat);
		void SetMatrix4(const String& name, const Matrix4& mat);

		void SetTexture(const String& name, NVRHI::TextureHandle texture);
		void SetSampler(const String& name, NVRHI::SamplerHandle sampler);
		
		void SetKeyword(const String& name, bool value);
	};
}