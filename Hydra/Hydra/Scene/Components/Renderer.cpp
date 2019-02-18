#include "Hydra/Scene/Components/Renderer.h"

#include <iostream>
#include "Hydra/Render/Mesh.h"

namespace Hydra
{

	Renderer::Renderer() : _RenderInterface(nullptr), _IndexHandle(nullptr), _VertexBuffer(nullptr), _InstBuffer(nullptr), _Mesh(nullptr), _IsInstanced(false), _LastInstanceCount(0), _NeedsUpdate(true), _NeedsUpdateInstances(true)
	{
		
	}

	Renderer::~Renderer()
	{
		if (_RenderInterface == nullptr) return;

		if (_IndexHandle != nullptr)
		{
			_RenderInterface->destroyBuffer(_IndexHandle);
		}

		if (_VertexBuffer != nullptr)
		{
			_RenderInterface->destroyBuffer(_VertexBuffer);
		}

		if (_InstBuffer != nullptr)
		{
			_RenderInterface->destroyBuffer(_InstBuffer);
		}
	}

	void Renderer::Start()
	{

	}

	void Renderer::Update()
	{

	}

	void Renderer::SetMesh(Mesh* mesh)
	{
		_Mesh = mesh;
		_NeedsUpdate = true;
	}

	Mesh* Renderer::GetMesh()
	{
		return _Mesh;
	}

	void Renderer::WriteMeshData(NVRHI::IRendererInterface* renderInterface)
	{
		if (!_NeedsUpdate) return;
		_NeedsUpdate = false;
		_RenderInterface = renderInterface;

		if (_IndexHandle != nullptr)
		{
			renderInterface->destroyBuffer(_IndexHandle);
		}
		if (_VertexBuffer != nullptr)
		{
			renderInterface->destroyBuffer(_VertexBuffer);
		}

		if (_Mesh == nullptr) return;
		if (_Mesh->Vertices.size() == 0 || _Mesh->Indices.size() == 0) return;

		List<VertexBufferEntry> VertexData;

		size_t vertexCount = _Mesh->Vertices.size();
		size_t uvsCount = _Mesh->TexCoords.size();
		size_t normalCount = _Mesh->Normals.size();
		size_t tangentCount = _Mesh->Tangents.size();
		size_t binormalCount = _Mesh->BiNormals.size();

		bool useUvs = uvsCount == vertexCount;
		bool useNormals = normalCount == vertexCount;
		bool useTangents = tangentCount == vertexCount;
		bool useBiNormals = binormalCount = vertexCount;



		NVRHI::BufferDesc indexBufferDesc;
		indexBufferDesc.isIndexBuffer = true;
		indexBufferDesc.byteSize = uint32_t(_Mesh->Indices.size() * sizeof(unsigned int));
		_IndexHandle = renderInterface->createBuffer(indexBufferDesc, &_Mesh->Indices[0]);

		for (int i = 0; i < vertexCount; i++)
		{
			VertexBufferEntry entry;

			entry.position = _Mesh->Vertices[i];

			if (useUvs)
			{
				entry.texCoord = _Mesh->TexCoords[i];
			}
			
			if (useNormals)
			{
				entry.normal = _Mesh->Normals[i];
			}
			
			if (useTangents)
			{
				entry.tangent = _Mesh->Tangents[i];
			}
			
			if (useBiNormals)
			{
				entry.binormal = _Mesh->BiNormals[i];
			}

			VertexData.push_back(entry);
		}

		NVRHI::BufferDesc vertexBufferDesc;
		vertexBufferDesc.isVertexBuffer = true;
		vertexBufferDesc.byteSize = uint32_t(VertexData.size() * sizeof(VertexBufferEntry));
		_VertexBuffer = renderInterface->createBuffer(vertexBufferDesc, &VertexData[0]);

		_DrawArguments.instanceCount = 1;
		_DrawArguments.startIndexLocation = 0;
		_DrawArguments.startInstanceLocation = 0;
		_DrawArguments.startVertexLocation = 0;
		_DrawArguments.vertexCount = _Mesh->Indices.size();

		//std::cout << "WriteMeshData (" << ToString(VertexData.size()) << ", " + ToString(_DrawArguments.vertexCount) + ")" << std::endl;
	}

	void Renderer::UpdateInstancing(NVRHI::IRendererInterface* renderInterface, NVRHI::DrawCallState& state)
	{
		/*if (_LastInstanceCount != InstanceObjects.size() || _NeedsUpdateInstances)
		{
			_LastInstanceCount = InstanceObjects.size();
			_NeedsUpdateInstances = false;

			if (_InstBuffer != nullptr)
			{
				_RenderInterface->destroyBuffer(_InstBuffer);
				_InstBuffer = nullptr;
			}

			Instances.clear();
			for (Transformable& obj : InstanceObjects)
			{
				Instances.push_back(obj.getModelMatrix());
			}

			if (_lastInstanceCount > 0)
			{
				NVRHI::BufferDesc instBufferDesc;
				instBufferDesc.isVertexBuffer = true;
				instBufferDesc.byteSize = uint32_t(Instances.size() * sizeof(glm::mat4));
				instBufferDesc.isCPUWritable = true;
				_instBuffer = renderInterface->createBuffer(instBufferDesc, &Instances[0]);

				for (MeshDataPart& part : _meshParts)
				{
					part.setInstanceCount(_lastInstanceCount);
				}
			}
			else
			{
				_isInstanced = false;
			}
		}

		if (_isInstanced)
		{
			state.vertexBufferCount = 2;
			state.vertexBuffers[1].buffer = _instBuffer;
			state.vertexBuffers[1].slot = 1;
			state.vertexBuffers[1].stride = sizeof(glm::mat4);
			//state.vertexBuffers[1].stride = sizeof(glm::vec4);
		}
		else
		{
			state.vertexBufferCount = 1;
		}*/
		state.vertexBufferCount = 1;
	}

	void Renderer::WriteDataToState(NVRHI::IRendererInterface* renderInterface, NVRHI::DrawCallState& state)
	{
		WriteMeshData(renderInterface);
		state.primType = NVRHI::PrimitiveType::TRIANGLE_LIST;
		/*if (_Mesh != nullptr)
		{
			state.primType = _Mesh->PrimitiveType;
		}*/

		state.indexBufferFormat = NVRHI::Format::R32_UINT;
		state.indexBuffer = _IndexHandle;

		state.vertexBufferCount = 1;
		state.vertexBuffers[0].buffer = _VertexBuffer;
		state.vertexBuffers[0].slot = 0;
		state.vertexBuffers[0].stride = sizeof(VertexBufferEntry);

		UpdateInstancing(renderInterface, state);
	}
	NVRHI::DrawArguments& Renderer::GetDrawArguments()
	{
		return _DrawArguments;
	}
}