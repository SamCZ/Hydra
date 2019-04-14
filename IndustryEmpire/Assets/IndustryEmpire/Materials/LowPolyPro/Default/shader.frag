#version 330 core

struct DirLight {
    vec3 Direction;
    vec3 Position;
    vec3 Color;
    float Intensity;
	bool HasShadowMap;
    sampler2D ShadowMap;
	mat4 ShadowProjection;
};

struct PointLight {
    vec3 Position;
    vec3 Color;
    float Radius;
    bool HasShadowMap;
    samplerCube ShadowMap;
};

out vec4 FragColor;

in vec2 TexCoords;
in vec3 ViewNormal;
in vec3 Normal;
in vec4 WorldPos;
in vec3 ColorModifier;

uniform vec3 ViewPos;

uniform DirLight DirLights[16];
uniform int DirLightSize;
uniform PointLight PointLights[64];
uniform int PointLightCount;
uniform vec4 AmbientLight = vec4(0.0);

uniform sampler2D ColorMap;
uniform sampler2D AOMap;

vec3 sampleOffsetDirections[20] = vec3[] (
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

float ShadowCalculation(vec3 fragPos, vec3 lightPos, samplerCube depthMap) {
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    /*float closestDepth = texture(depthMap, fragToLight).r;
    closestDepth *= 500.0;
    float currentDepth = length(fragToLight);
    float bias = 0.05; 
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

    return shadow;*/

    float shadow = 0.0;
    float bias   = 0.1;
    int samples  = 1;
    float far_plane = 500.0;
    float viewDistance = length(ViewPos - fragPos);
    //float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    float diskRadius = 0.005;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= far_plane;
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);

    return shadow;
}

float ComputeDirLightShadow() {
	if(!DirLights[0].HasShadowMap) return 1.0;

	float shadowValue = 0.0;

	vec4 mappedCoords = DirLights[0].ShadowProjection * vec4(WorldPos.xyz, 1.0);
	vec3 shadowCoords = (mappedCoords.xyz / mappedCoords.w) * 0.5 + 0.5;

	float closestDepth = texture(DirLights[0].ShadowMap, shadowCoords.xy).r;

	float bias = 0.0001;

	//float bias = max(0.05 * (1.0 - dot(Normal, DirLights[0].Direction)), 0.005);  

	/*if(shadowCoords.z - bias > closestDepth) {
		shadowValue += 1.0;
	}*/

	float currentDepth = shadowCoords.z;

	shadowValue = (currentDepth - bias) > closestDepth  ? 0.0 : 1.0;

	if(shadowCoords.x < 0 || shadowCoords.x > 1 || shadowCoords.y < 0 || shadowCoords.y > 1) {
		return 1.0;
	} else {
		return shadowValue;
	}
}

void main() {
	vec3 norm = normalize(Normal);

	float shadowValue = ComputeDirLightShadow();

	float aoValue = texture(AOMap, vec2(TexCoords.s, 1.0 - TexCoords.t)).r;
	vec3 objectColor = texture(ColorMap, vec2(TexCoords.s, 1.0 - TexCoords.t)).rgb * aoValue;

	/*
	for(int pl = 0; pl < PointLightCount; pl++) {
		shadowValue += 1.0 - ShadowCalculation(vec3(WorldPos), PointLights[pl].Position, PointLights[pl].ShadowMap);
	}*/

	float diff = max(dot(norm, DirLights[0].Direction), 0.2);
	vec3 diffuse = diff * DirLights[0].Color * DirLights[0].Intensity * shadowValue;

	vec3 result = (AmbientLight.rgb + diffuse) * objectColor;
	FragColor = vec4(result, 1.0);
}