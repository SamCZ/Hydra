#pragma once

#include "Hydra/Scene/Component.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"
#include "Hydra/Core/Vector.h"

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
		NVRHI::IRendererInterface* _RenderInterface;
		NVRHI::BufferHandle _IndexHandle;
		NVRHI::BufferHandle _VertexBuffer;
		NVRHI::BufferHandle _InstBuffer;

		NVRHI::DrawArguments _DrawArguments;

		Mesh* _Mesh;

		bool _NeedsUpdate;
		bool _IsInstanced;
		bool _NeedsUpdateInstances;
		int _LastInstanceCount;
	public:
		Renderer();
		~Renderer();

		virtual void Start();
		virtual void Update();

		void SetMesh(Mesh* mesh);
		Mesh* GetMesh();

		//TODO: Instancing

	private:
		void WriteMeshData(NVRHI::IRendererInterface* renderInterface);
		void UpdateInstancing(NVRHI::IRendererInterface* renderInterface, NVRHI::DrawCallState& state);

	public:
		void WriteDataToState(NVRHI::IRendererInterface* renderInterface, NVRHI::DrawCallState& state);

		NVRHI::DrawArguments& GetDrawArguments();
	};
}