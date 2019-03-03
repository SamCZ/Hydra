#include "Hydra/Scene/Components/Renderer.h"

#include <iostream>
#include "Hydra/Render/Mesh.h"
#include "Hydra/Core/Log.h"

namespace Hydra
{

	Renderer::Renderer() : _RenderInterface(nullptr), _IndexHandle(nullptr), _VertexBuffer(nullptr), _InstBuffer(nullptr), _Mesh(nullptr), _IsInstanced(false), _LastInstanceCount(0), _NeedsUpdate(true), _NeedsUpdateInstances(true)
	{
		Mat.Albedo = nullptr;
		Mat.Normal = nullptr;
		Mat.Roughness = nullptr;
		Mat.Metallic = nullptr;
		Mat.Opacity = nullptr;
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

		if (_Mesh != nullptr)
		{
			delete _Mesh;
		}

		if (Mat.Albedo)
		{
			_RenderInterface->destroyTexture(Mat.Albedo);
		}

		if (Mat.Normal)
		{
			_RenderInterface->destroyTexture(Mat.Normal);
		}

		if (Mat.Metallic)
		{
			_RenderInterface->destroyTexture(Mat.Metallic);
		}

		if (Mat.Roughness)
		{
			_RenderInterface->destroyTexture(Mat.Roughness);
		}

		if (Mat.Opacity)
		{
			_RenderInterface->destroyTexture(Mat.Opacity);
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

	void Renderer::AddInstance(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz)
	{
		_IsInstanced = true;
		Transformable obj;
		obj.Position = Vector3(x, y, z);
		obj.Rotation = Vector3(rx, ry, rz);
		obj.Scale = Vector3(sx, sy, sz);
		_InstanceObjects.push_back(obj);
		_InstanceData.push_back(obj.GetModelMatrix());
	}

	void Renderer::AddInstance(const Vector3 & pos, const Vector3 & rotation, const Vector3 & scale)
	{
		_IsInstanced = true;
		Transformable obj;
		obj.Position = pos;
		obj.Rotation = rotation;
		obj.Scale = scale;
		_InstanceObjects.push_back(obj);
		_InstanceData.push_back(obj.GetModelMatrix());
	}

	void Renderer::RemoveAllInstances()
	{
		_IsInstanced = false;
		_InstanceObjects.clear();
		_InstanceData.clear();
	}

	void Renderer::WriteMeshData()
	{
		if (!_NeedsUpdate) return;
		_NeedsUpdate = false;
		_RenderInterface = Engine::GetRenderInterface();

		if (_IndexHandle != nullptr)
		{
			_RenderInterface->destroyBuffer(_IndexHandle);
		}
		if (_VertexBuffer != nullptr)
		{
			_RenderInterface->destroyBuffer(_VertexBuffer);
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
		bool useBiNormals = binormalCount == vertexCount;

		if (!useUvs)
		{
			Log("Renderer::WriteMeshData", "Mesh(" + _Mesh->GetSource() + ") has zero UVs that can broke lighting because UVs are needed for tange space callculation !");
		}

		//Log("Renderer::WriteMeshData", "Mesh(" + _Mesh->GetSource() + ") UV: " + ToString(useUvs) + ", N: " + ToString(useNormals) + ", TA: " + ToString(useTangents) + ", BITA: " + ToString(useBiNormals));

		NVRHI::BufferDesc indexBufferDesc;
		indexBufferDesc.isIndexBuffer = true;
		indexBufferDesc.byteSize = uint32_t(_Mesh->Indices.size() * sizeof(unsigned int));
		_IndexHandle = _RenderInterface->createBuffer(indexBufferDesc, &_Mesh->Indices[0]);

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
		_VertexBuffer = _RenderInterface->createBuffer(vertexBufferDesc, &VertexData[0]);

		_DrawArguments.instanceCount = 1;
		_DrawArguments.startIndexLocation = 0;
		_DrawArguments.startInstanceLocation = 0;
		_DrawArguments.startVertexLocation = 0;
		_DrawArguments.vertexCount = static_cast<uint32_t>(_Mesh->Indices.size());

		//std::cout << "WriteMeshData (" << ToString(VertexData.size()) << ", " + ToString(_DrawArguments.vertexCount) + ")" << std::endl;
	}

	void Renderer::UpdateInstancing(NVRHI::DrawCallState& state)
	{
		if (_LastInstanceCount != _InstanceData.size() || _NeedsUpdateInstances)
		{
			_LastInstanceCount = _InstanceData.size();
			_NeedsUpdateInstances = false;

			if (_InstBuffer != nullptr)
			{
				_RenderInterface->destroyBuffer(_InstBuffer);
				_InstBuffer = nullptr;
			}

			if (_LastInstanceCount > 0)
			{
				NVRHI::BufferDesc instBufferDesc;
				instBufferDesc.isVertexBuffer = true;
				instBufferDesc.byteSize = uint32_t(_InstanceData.size() * sizeof(Matrix4));
				instBufferDesc.isCPUWritable = true;
				_InstBuffer = _RenderInterface->createBuffer(instBufferDesc, &_InstanceData[0]);

				_DrawArguments.instanceCount = _LastInstanceCount;
			}
			else
			{
				_IsInstanced = false;
				_DrawArguments.instanceCount = 1;
			}
		}

		if (_IsInstanced)
		{
			state.vertexBufferCount = 2;
			state.vertexBuffers[1].buffer = _InstBuffer;
			state.vertexBuffers[1].slot = 1;
			state.vertexBuffers[1].stride = sizeof(glm::mat4);
			//state.vertexBuffers[1].stride = sizeof(glm::vec4);
		}
		else
		{
			state.vertexBufferCount = 1;
		}
	}

	void Renderer::WriteDataToState(NVRHI::DrawCallState& state)
	{
		WriteMeshData();
		//state.primType = NVRHI::PrimitiveType::;
		if (_Mesh != nullptr)
		{
			state.primType = _Mesh->PrimitiveType;
		}

		state.indexBufferFormat = NVRHI::Format::R32_UINT;
		state.indexBuffer = _IndexHandle;

		state.vertexBufferCount = 1;
		state.vertexBuffers[0].buffer = _VertexBuffer;
		state.vertexBuffers[0].slot = 0;
		state.vertexBuffers[0].stride = sizeof(VertexBufferEntry);

		UpdateInstancing(state);
	}
	NVRHI::DrawArguments& Renderer::GetDrawArguments()
	{
		return _DrawArguments;
	}
}