#pragma hydra cmp:CSMain

struct VoxelBuffer
{
	float4 Position;
	float3 Normal;
};

struct CollisionBuffer
{
	int Result;
	float3 V0;
	float3 V1;
	float3 V2;
	float3 Normal;
};

cbuffer Params
{
	float3 _Position;
	float _Radius;
};

StructuredBuffer<VoxelBuffer> _Buffer;
RWStructuredBuffer<CollisionBuffer> _Collisions;

#define FLT_EPSILON 0.0000001

bool Intersects(float3 origin, float3 direction, float3 v0, float3 v1, float3 v2, out float dist)
{
	float edge1X = v1.x - v0.x;
	float edge1Y = v1.y - v0.y;
	float edge1Z = v1.z - v0.z;

	float edge2X = v2.x - v0.x;
	float edge2Y = v2.y - v0.y;
	float edge2Z = v2.z - v0.z;

	float normX = ((edge1Y * edge2Z) - (edge1Z * edge2Y));
	float normY = ((edge1Z * edge2X) - (edge1X * edge2Z));
	float normZ = ((edge1X * edge2Y) - (edge1Y * edge2X));

	float dirDotNorm = direction.x * normX + direction.y * normY + direction.z * normZ;

	float diffX = origin.x - v0.x;
	float diffY = origin.y - v0.y;
	float diffZ = origin.z - v0.z;

	float sign;
	if (dirDotNorm > FLT_EPSILON)
	{
		sign = 1;
	}
	else if (dirDotNorm < -FLT_EPSILON)
	{
		sign = -1.0f;
		dirDotNorm = -dirDotNorm;
	}
	else
	{
		// ray and triangle/quad are parallel
		dist = 0;
		return false;
	}

	float diffEdge2X = ((diffY * edge2Z) - (diffZ * edge2Y));
	float diffEdge2Y = ((diffZ * edge2X) - (diffX * edge2Z));
	float diffEdge2Z = ((diffX * edge2Y) - (diffY * edge2X));

	float dirDotDiffxEdge2 = sign * (direction.x * diffEdge2X
		+ direction.y * diffEdge2Y
		+ direction.z * diffEdge2Z);

	if (dirDotDiffxEdge2 >= 0.0f)
	{
		diffEdge2X = ((edge1Y * diffZ) - (edge1Z * diffY));
		diffEdge2Y = ((edge1Z * diffX) - (edge1X * diffZ));
		diffEdge2Z = ((edge1X * diffY) - (edge1Y * diffX));

		float dirDotEdge1xDiff = sign * (direction.x * diffEdge2X
			+ direction.y * diffEdge2Y
			+ direction.z * diffEdge2Z);

		if (dirDotEdge1xDiff >= 0.0f)
		{
			if (dirDotDiffxEdge2 + dirDotEdge1xDiff <= dirDotNorm)
			{
				float diffDotNorm = -sign * (diffX * normX + diffY * normY + diffZ * normZ);
				if (diffDotNorm >= 0.0f)
				{
					// ray intersects triangle
					// fill in.
					/*float inv = 1.0f / dirDotNorm;
					float t = diffDotNorm * inv;
					dist = t;*/
					dist = 0;
					return true;
				}
			}
		}
	}

	return false;
}

bool IntersectionSphereTriangle(float3 sphereCenter, float sphereRadius, float3 a, float3 b, float3 c, float3 normal)
{
	//if (CmpVector(a, b) && CmpVector(a, c)) return false;

	float dist;

	return Intersects(sphereCenter, normalize(float3(0, -1, 0)), a, b, c, dist);
}

[numthreads(512, 1, 1)]
void CSMain(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	_Collisions[0].Result = 0;

	int i = dispatchThreadId.x / 3;


	VoxelBuffer v0 = _Buffer[i * 3 + 0];
	VoxelBuffer v1 = _Buffer[i * 3 + 1];
	VoxelBuffer v2 = _Buffer[i * 3 + 2];


	if (IntersectionSphereTriangle(_Position, _Radius, v0.Position.xyz, v1.Position.xyz, v2.Position.xyz, float3(0, 1, 0)))
	{
		_Collisions[0].Result = 1;

		_Collisions[0].V0 = v0.Position.xyz;
		_Collisions[0].V1 = v1.Position.xyz;
		_Collisions[0].V2 = v2.Position.xyz;

		return;
	}
}