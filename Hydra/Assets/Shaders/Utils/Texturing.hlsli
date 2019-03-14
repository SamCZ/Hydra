float3 GetTriaplanarBlend(float3 normal)
{
	float3 blending = abs(normal) - 0.2679f;
	blending = normalize(max(blending, 0.0));
	return blending / (blending.x + blending.y + blending.z).xxx;
}

float4 TriplanarTexturing(Texture2D tex, SamplerState smplr, float3 worldPos, float3 normal, float scale)
{
	float3 blendAxes = GetTriaplanarBlend(normalize(normal));
	float3 scaledWorldPos = worldPos / scale;
	float4 xProjection = tex.Sample(smplr, scaledWorldPos.yz) * blendAxes.x;
	float4 yProjection = tex.Sample(smplr, scaledWorldPos.xz) * blendAxes.y;
	float4 zProjection = tex.Sample(smplr, scaledWorldPos.xy) * blendAxes.z;
	float4 color = (xProjection + yProjection + zProjection) / 3.0;

	return color;
}

float3 BlendNormals(float3 normal0, float3 normal1)
{
	return normalize(float3(normal0.r + normal1.r, normal0.g * normal1.g, normal0.b + normal1.b));
}

float3 TriplanarTexturingNormal(Texture2D tex, SamplerState smplr, float3 worldPos, float3 normal, float scale)
{
	float3 blendAxes = GetTriaplanarBlend(normalize(normal));
	float3 scaledWorldPos = worldPos / scale;
	float4 xProjection = tex.Sample(smplr, scaledWorldPos.yz) * blendAxes.x;
	float4 yProjection = tex.Sample(smplr, scaledWorldPos.xz) * blendAxes.y;
	float4 zProjection = tex.Sample(smplr, scaledWorldPos.xy) * blendAxes.z;
	
	return BlendNormals(BlendNormals(xProjection.rgb, yProjection.rgb), zProjection.rgb);
}