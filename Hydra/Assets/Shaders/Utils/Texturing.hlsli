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
	
	return xProjection + yProjection + zProjection;

	float4 color = (xProjection + yProjection + zProjection) / 3.0;

	return xProjection;

	color.a = 1.0;

	return color;
}

float3 BlendNormals(float3 normal0, float3 normal1)
{
	return normalize(float3(normal0.r + normal1.r, normal0.g * normal1.g, normal0.b + normal1.b));
}
/*
float3 GetNormalFromNormalMap(PS_Input input, Texture2D tex, SamplerState smplr, float2 texCoords)
{
	float3 texColorNormal = tex.Sample(smplr, texCoords).rgb;
	texColorNormal = normalize(texColorNormal * 2.0 - 1.0);

	float3 tangent = normalize(input.tangent);
	float3 bitangent = normalize(input.bitangent);
	float3x3 TangentMatrix = float3x3(tangent, bitangent, input.normal);

	return -normalize(mul(TangentMatrix, texColorNormal));
}

float3 TriplanarTexturingNormal(PS_Input input, Texture2D tex, SamplerState smplr, float scale)
{
	float3 blendAxes = GetTriaplanarBlend(normalize(input.normal));
	float3 scaledWorldPos = input.positionWS / scale;
	float3 xProjection = GetNormalFromNormalMap(input, tex, smplr, scaledWorldPos.yz) * blendAxes.x;
	float3 yProjection = GetNormalFromNormalMap(input, tex, smplr, scaledWorldPos.xz) * blendAxes.y;
	float3 zProjection = GetNormalFromNormalMap(input, tex, smplr, scaledWorldPos.xy) * blendAxes.z;
	
	return normalize((xProjection + yProjection + zProjection) * 0.3333333333);
}*/