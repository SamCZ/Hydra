#pragma hydra cmp:CSMain

struct VoxelBuffer
{
	float4 Position;
	float3 Normal;
};

cbuffer Params
{
	float _Target;
	int _Width;
	int _Height;
	int _Depth;
	int _Border;
};


RWStructuredBuffer<VoxelBuffer> _Buffer;

StructuredBuffer<int> _TriangleConnectionTable;
StructuredBuffer<int> _CubeEdgeFlags;
StructuredBuffer<float> _Voxels;

Texture3D<float4> _Normals;

sampler _LinearClamp 
{
	addressU = Clamp;
	addressV = Clamp;
	AddressW = Clamp;
	mipfilter = NONE;
	minfilter = LINEAR;
	magfilter = LINEAR;
};

#define CMP_TRI_NORMALS 0

// edgeConnection lists the index of the endpoint vertices for each of the 12 edges of the cube
static int2 edgeConnection[12] =
{
	int2(0,1), int2(1,2), int2(2,3), int2(3,0), int2(4,5), int2(5,6), int2(6,7), int2(7,4), int2(0,4), int2(1,5), int2(2,6), int2(3,7)
};

// edgeDirection lists the direction vector (vertex1-vertex0) for each edge in the cube
static float3 edgeDirection[12] =
{
	float3(1.0f, 0.0f, 0.0f),float3(0.0f, 1.0f, 0.0f),float3(-1.0f, 0.0f, 0.0f),float3(0.0f, -1.0f, 0.0f),
	float3(1.0f, 0.0f, 0.0f),float3(0.0f, 1.0f, 0.0f),float3(-1.0f, 0.0f, 0.0f),float3(0.0f, -1.0f, 0.0f),
	float3(0.0f, 0.0f, 1.0f),float3(0.0f, 0.0f, 1.0f),float3(0.0f, 0.0f, 1.0f),float3(0.0f,  0.0f, 1.0f)
};

// vertexOffset lists the positions, relative to vertex0, of each of the 8 vertices of a cube
static float3 vertexOffset[8] =
{
	float3(0, 0, 0),float3(1, 0, 0),float3(1, 1, 0),float3(0, 1, 0),
	float3(0, 0, 1),float3(1, 0, 1),float3(1, 1, 1),float3(0, 1, 1)
};

void FillCube(int x, int y, int z, out float cube[8])
{
	cube[0] = _Voxels[x + y * _Width + z * _Width * _Height];
	cube[1] = _Voxels[(x + 1) + y * _Width + z * _Width * _Height];
	cube[2] = _Voxels[(x + 1) + (y + 1) * _Width + z * _Width * _Height];
	cube[3] = _Voxels[x + (y + 1) * _Width + z * _Width * _Height];

	cube[4] = _Voxels[x + y * _Width + (z + 1) * _Width * _Height];
	cube[5] = _Voxels[(x + 1) + y * _Width + (z + 1) * _Width * _Height];
	cube[6] = _Voxels[(x + 1) + (y + 1) * _Width + (z + 1) * _Width * _Height];
	cube[7] = _Voxels[x + (y + 1) * _Width + (z + 1) * _Width * _Height];
}

// GetOffset finds the approximate point of intersection of the surface
// between two points with the values v1 and v2
float GetOffset(float v1, float v2)
{
	float delta = v2 - v1;
	return (delta == 0.0f) ? 0.5f : (_Target - v1) / delta;
}

VoxelBuffer CreateVertex(float3 position, float3 centre, float3 size
#if CMP_TRI_NORMALS
	, float3 triNorm
#endif
)
{
	VoxelBuffer vert;
	vert.Position = float4(position - centre, 1.0);

	float3 uv = position / size;
	vert.Normal = _Normals.SampleLevel(_LinearClamp, uv, 0).xyz;

#if CMP_TRI_NORMALS
	vert.Normal = triNorm;
#endif

	return vert;
}

#if CMP_TRI_NORMALS
float3 ComputeTriangleNormal(float3 p1, float3 p2, float3 p3)
{
	float3 U = p2 - p1;
	float3 V = p3 - p1;
	float x = (U.y * V.z) - (U.z * V.y);
	float y = (U.z * V.x) - (U.x * V.z);
	float z = (U.x * V.y) - (U.y * V.x);
	return normalize(float3(x, y, z));
}
#endif

[numthreads(8, 8, 8)]
void CSMain(int3 id : SV_DispatchThreadID)
{
	//Dont generate verts at the edge as they dont have 
	//neighbours to make a cube from and the normal will 
	//not be correct around border.
	if (id.x >= _Width - 1 - _Border) return;
	if (id.y >= _Height - 1 - _Border) return;
	if (id.z >= _Depth - 1 - _Border) return;

	float3 pos = float3(id);
	float3 centre = float3(_Width, 0, _Depth) / 2.0;

	float cube[8];
	FillCube(id.x, id.y, id.z, cube);

	int i = 0;
	int flagIndex = 0;
	float3 edgeVertex[12];

	//Find which vertices are inside of the surface and which are outside
	for (i = 0; i < 8; i++)
		if (cube[i] <= _Target) flagIndex |= 1 << i;

	//Find which edges are intersected by the surface
	int edgeFlags = _CubeEdgeFlags[flagIndex];

	// no connections, return
	if (edgeFlags == 0) return;

	//Find the point of intersection of the surface with each edge
	for (i = 0; i < 12; i++)
	{
		//if there is an intersection on this edge
		if ((edgeFlags & (1 << i)) != 0)
		{
			float offset = GetOffset(cube[edgeConnection[i].x], cube[edgeConnection[i].y]);

			edgeVertex[i] = pos + (vertexOffset[edgeConnection[i].x] + offset * edgeDirection[i]);
		}
	}

	float3 size = float3(_Width - 1, _Height - 1, _Depth - 1);

	int idx = id.x + id.y * _Width + id.z * _Width * _Height;

	//Save the triangles that were found. There can be up to five per cube
	for (i = 0; i < 5; i++)
	{
		//If the connection table is not -1 then this a triangle.
		if (_TriangleConnectionTable[flagIndex * 16 + 3 * i] >= 0)
		{
#if CMP_TRI_NORMALS
			float3 pos0 = edgeVertex[_TriangleConnectionTable[flagIndex * 16 + (3 * i + 0)]];
			float3 pos1 = edgeVertex[_TriangleConnectionTable[flagIndex * 16 + (3 * i + 1)]];
			float3 pos2 = edgeVertex[_TriangleConnectionTable[flagIndex * 16 + (3 * i + 2)]];

			float3 norm = ComputeTriangleNormal(pos0, pos1, pos2);

			_Buffer[idx * 15 + (3 * i + 0)] = CreateVertex(pos0, centre, size, norm);
			_Buffer[idx * 15 + (3 * i + 1)] = CreateVertex(pos1, centre, size, norm);
			_Buffer[idx * 15 + (3 * i + 2)] = CreateVertex(pos2, centre, size, norm);
#else
			float3 position;

			position = edgeVertex[_TriangleConnectionTable[flagIndex * 16 + (3 * i + 0)]];
			_Buffer[idx * 15 + (3 * i + 0)] = CreateVertex(position, centre, size);

			position = edgeVertex[_TriangleConnectionTable[flagIndex * 16 + (3 * i + 1)]];
			_Buffer[idx * 15 + (3 * i + 1)] = CreateVertex(position, centre, size);

			position = edgeVertex[_TriangleConnectionTable[flagIndex * 16 + (3 * i + 2)]];
			_Buffer[idx * 15 + (3 * i + 2)] = CreateVertex(position, centre, size);
#endif
		}
	}

}