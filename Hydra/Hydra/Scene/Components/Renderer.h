#pragma once

#include "Hydra/Scene/Transformable.h"
#include "Hydra/Scene/Component.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"
#include "Hydra/Core/Vector.h"
#include "Hydra/Core/Container.h"
#include "Hydra/Engine.h"

namespace Hydra
{
	struct VertexBufferEntry
	{
		glm::vec3 position;
		glm::vec2 texCoord;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 binormal;
	};

	class Mesh;

	class Renderer : public Component
	{
	private:
		IRendererInterface _RenderInterface;
		NVRHI::BufferHandle _IndexHandle;
		NVRHI::BufferHandle _VertexBuffer;
		NVRHI::BufferHandle _InstBuffer;

		List<Transformable> _InstanceObjects;
		List<Matrix4> _InstanceData;

		NVRHI::DrawArguments _DrawArguments;

		Mesh* _Mesh;

		bool _NeedsUpdate;
		bool _IsInstanced;
		bool _NeedsUpdateInstances;
		int _LastInstanceCount;
	public:
		Renderer();
		~Renderer();

		glm::vec3 TestColor;

		virtual void Start();
		virtual void Update();

		void SetMesh(Mesh* mesh);
		Mesh* GetMesh();

		void AddInstance(float x, float y, float z, float rx = 0.0f, float ry = 0.0f, float rz = 0.0, float sx = 1.0f, float sy = 1.0, float sz = 1.0);
		void AddInstance(const Vector3& pos, const Vector3& rotation = Vector3(), const Vector3& scale = Vector3(1.0f));
		void RemoveAllInstances();

		//TODO: Instancing

	private:
		void WriteMeshData();
		void UpdateInstancing(NVRHI::DrawCallState& state);

	public:
		void WriteDataToState(NVRHI::DrawCallState& state);

		NVRHI::DrawArguments& GetDrawArguments();
	};

	DEFINE_PTR(Renderer)
}